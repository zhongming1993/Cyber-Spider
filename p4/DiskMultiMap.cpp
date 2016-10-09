//
//  DiskMultiMap.cpp
//  p4
//
//  Created by MingZhong on 3/7/16.
//  Copyright © 2016 MingZhong. All rights reserved.
//

#include "DiskMultiMap.h"
#include <functional>

/*
void DiskMultiMap::printAll()
{
    int num_of_b;
    my_file.read(num_of_b, 0);
    cout << "number of bucket is : " << num_of_b << endl;
    int next_loc;
    my_file.read(next_loc, sizeof(unsigned int));
    cout << "initial next_loc is : (initally should be a random number)" << next_loc << endl;
    BinaryFile::Offset reused_location;
    my_file.read(reused_location, my_location(num_of_buckets));
    cout << "reused location is : " << reused_location << "    ";
    while (reused_location)
    {
        DiskNode node;
        my_file.read(node, reused_location);
        cout << node.next << "   ";
        reused_location = node.next;
    }
    cout << endl;
    for (int i = 0; i < num_of_buckets; i++)
    {
        cout << "bucket[" << i << "]:" << endl;
        bucket my_bucket;
        DiskNode node;
        my_file.read(my_bucket, my_location(i));
        BinaryFile::Offset index = my_bucket.head;
        while (index)
        {
            cout << "Disk Location: " << index << "   ";
            my_file.read(node, index);
            cout << node.my_key << "   " << node.my_value << "    " << node.my_context << endl;
            index = node.next;
        }
    }
}
*/

DiskMultiMap::Iterator::Iterator()
{
    my_position = 0;
    parent = nullptr;
}

DiskMultiMap::Iterator::Iterator(DiskMultiMap* x, BinaryFile::Offset position):parent(x),my_position(position){}

DiskMultiMap::Iterator::~Iterator()
{
    my_position = 0;
}

bool DiskMultiMap::Iterator::isValid() const
{
    if (my_position == 0)
        return false;
    else return true;
}

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++()
{
    // if the iterator is invalid
    if (!this -> isValid())
        return (*this);
    else
    {
        MultiMapTuple tuple = *(*this);
        DiskNode node;
        if (!parent -> my_file.read(node, my_position))
            cerr << " read node in Iterator's operator++ error" << endl;
        BinaryFile::Offset current_location = node.next;
        while (current_location)
        {
            DiskNode current_node;
            if (!parent -> my_file.read(current_node, current_location))
                cerr << " read node in Iterator's operator++ error" << endl;
            // find another node that matchs key
            if (strcmp(current_node.my_key, tuple.key.c_str()) == 0)
            {
                my_position = current_location;
                return (*this);
            }
            else current_location = current_node.next;
        }
        // set the iterator to invalid
        my_position = 0;
        return (*this);
    }
}

MultiMapTuple DiskMultiMap::Iterator::operator*()
{
    MultiMapTuple tuple;
    // if the iterator is invalid
    if (!this -> isValid())
    {
        tuple.key.clear();
        tuple.value.clear();
        tuple.context.clear();
    }
    else
    {
        DiskNode node;
        if (!parent->my_file.read(node, this -> my_position))
            cerr << "read node in Iterator's operator*() error" << endl;
        tuple.key = node.my_key;
        tuple.value = node.my_value;
        tuple.context = node.my_context;
    }
    return tuple;
}

DiskMultiMap::DiskMultiMap()
{
    num_of_buckets = 0;
    next_available_memory = 0;
}

DiskMultiMap::~DiskMultiMap()
{
    close();
}

// initialize the binaryfile, fill in the file with num_of_buckets of buckets from address 0
// each of the bucket contains a head(initialized as 0) for its  corresponding linkedlist
// thus, for this binary file,
// 0 ~ sizeof(unsigned int)-1 stores an unsigned int num_of_buckets;
// sizeof(unsigned int) ~ sizeof(unsigned int)+sizeof(BinaryFile::Offset)-1 stores next_available_memory;
// sizeof(unsigned int)+sizeof(BinaryFile::Offset) - sizeof(unsigned int)+sizeof(BinaryFile::Offset)+ num_of_buckets*sizeof(bucket)-1 is occupied by the buckets
// the reused_location is stored at sizeof(unsigned int)+sizeof(BinaryFile::Offset)+num_of_buckets*sizeof(bucket)
// next_available_memory is sizeof(unsigned int)+sizeof(BinaryFile::Offset)+num_of_buckets*sizeof(bucket)+sizeof(BinaryFile::Offset) after this function.
void DiskMultiMap::initialize_hash_table()
{
    // initially, next_available_memory = 0
    my_file.write(num_of_buckets, next_available_memory);
    next_available_memory = next_available_memory + sizeof(unsigned int);
    // reserve the space for the variable next_available_memory
    next_available_memory = next_available_memory + sizeof(BinaryFile::Offset);
    for (int i = 0; i < num_of_buckets; i++)
    {
        my_file.write(bucket(i), next_available_memory);
        next_available_memory = next_available_memory + sizeof(bucket);
    }
    // this offset store the location where node is deleted recently.
    // if no node can be reused, then this value is 0;
    BinaryFile::Offset reused_location = 0;
    my_file.write(reused_location, next_available_memory);
    next_available_memory = next_available_memory +sizeof(BinaryFile::Offset);
}

BinaryFile::Offset DiskMultiMap::my_location(int i)
{
    // for reused_location, i = num_of_buckets.
    return sizeof(unsigned int)+sizeof(BinaryFile::Offset)+i*sizeof(bucket);
}


// create an empty, open hash table in binary disk with the specific filename and number of buckets.
bool DiskMultiMap::createNew(const std::string &filename, unsigned int numBuckets)
{
    num_of_buckets = numBuckets;
    // close the binary file is the object already opened a file without closing it.
    if (my_file.isOpen())
        my_file.close();
    // create a binary file successfully
    if (my_file.createNew(filename))
    {
        initialize_hash_table();
        return true;
    }
    // fail to create a binary file.
    else
        return false;
}

// open a previously-created disk-based hash table with the specified name.
bool DiskMultiMap::openExisting(const std::string &filename)
{
    if (my_file.isOpen())
        my_file.close();
    if (my_file.openExisting(filename))
    {
        if (!my_file.read(num_of_buckets,0))
            cerr << " read num_of_buckets error in DiskMultiMap's openExisting" << endl;
        if (!my_file.read(next_available_memory, sizeof(unsigned int)))
            cerr << " read next_available_memory error in DiskMultiMap's openExisting" << endl;
        return true;
    }
    else return false;
}

// close the currently used binary file
void DiskMultiMap::close()
{
    if (my_file.isOpen())
    {
        if (!my_file.write(num_of_buckets, 0))
            cerr << " write num_of_buckets error in DiskMultiMap's close()" << endl;
        if (!my_file.write(next_available_memory, sizeof(unsigned int)))
            cerr << " write next_availble_memory error in DiskMultiMap's close()" << endl;
        my_file.close();
    }
}

BinaryFile::Offset DiskMultiMap::find_available_location()
{
    BinaryFile::Offset reused_location;
    my_file.read(reused_location, my_location(num_of_buckets));
    if (reused_location == 0)
        return next_available_memory;
    else
    {
        // the node located at reused_location is override
        // update the value at reused_location
        DiskNode node;
        my_file.read(node, reused_location);
        my_file.write(node.next, my_location(num_of_buckets));
        return reused_location;
    }
}

unsigned int DiskMultiMap::hash_me(const std::string & str)
{
    std::hash<std::string> str_hash;
    unsigned int hashValue = str_hash(str);
    unsigned int hash_result = hashValue % num_of_buckets;
    return hash_result;
}

bool DiskMultiMap::insert(const std::string &key, const std::string &value, const std::string &context)
{
    // string length overflaw
    if (key.size() >= 120 || value.size() >= 120 || context.size() >= 120)
        return false;
    else
    {
        // insert the node to the front
        unsigned int my_bucket_id = hash_me(key);
        bucket my_bucket;
        if (!my_file.read(my_bucket, my_location(my_bucket_id)))
            cerr  << "read bucket in DistMultiMap.insert() error" << endl;
        BinaryFile::Offset my_head = my_bucket.head;
        DiskNode node(key.c_str(), value.c_str(), context.c_str(), my_head, 1);
        BinaryFile::Offset my_position = find_available_location();
        if (!my_file.write(node, my_position))
            cerr  << "write node in DistMultiMap.insert() error" << endl;
        my_bucket.head = my_position;
        my_file.write(my_bucket, my_location(my_bucket_id));
        if (my_position == next_available_memory)
            next_available_memory = next_available_memory + sizeof(DiskNode);
        return true;
    }
}

DiskMultiMap::Iterator DiskMultiMap::search(const std::string &key)
{
    // hash the key to the corresponding bucket.
    unsigned int my_bucket_id = hash_me(key);
    // find the corresponding head of this key.
    bucket my_bucket;
    if (!my_file.read(my_bucket, my_location(my_bucket_id)))
        cerr  << "read bucket in DistMultiMap.search() error" << endl;
    BinaryFile::Offset my_head = my_bucket.head;
    while (my_head)
    {
        DiskNode node;
        if (!my_file.read(node, my_head))
            cerr  << "read node in DistMultiMap.search() error" << endl;
        if (strcmp(node.my_key, key.c_str()) == 0)
        {
            DiskMultiMap::Iterator it(this, my_head);
            return it;
        }
        else my_head = node.next;
    }
    DiskMultiMap::Iterator is;
    return is;
}

int DiskMultiMap::erase(const std::string &key, const std::string &value, const std::string &context)
{
    int num_of_erase = 0;
    // hash the key to the corresponding bucket.
    unsigned int my_bucket_id = hash_me(key);
    // find the corresponding head of this key.
    bucket my_bucket;
    if (!my_file.read(my_bucket, my_location(my_bucket_id)))
        cerr  << "read bucket in DistMultiMap.search() error" << endl;
    BinaryFile::Offset my_head = my_bucket.head;
    BinaryFile::Offset reused_location;
    while (my_head != 0)
    {
        DiskNode node;
        my_file.read(node, my_head);
        // if this node need to be removed.
        if (strcmp(node.my_key, key.c_str()) == 0 && strcmp(node.my_value, value.c_str()) == 0 && strcmp(node.my_context, context.c_str()) == 0)
        {
            my_bucket.head = node.next;
            // update head.
            my_file.write(my_bucket, my_location(my_bucket_id));
            num_of_erase++;
            
            // update the node we want to delete as invalid.
            my_file.read(reused_location, my_location(num_of_buckets));
            node.next = reused_location;
            my_file.write(node, my_head);
            my_file.write(my_head, my_location(num_of_buckets));
            my_head = my_bucket.head;
        }
        else break;
    }
    BinaryFile::Offset current_node = my_head;
    while (current_node!=0)
    {
        DiskNode node;
        my_file.read(node, current_node);
        if (node.next!=0)
        {
            DiskNode next_node;
            my_file.read(next_node, node.next);
            if (strcmp(next_node.my_key, key.c_str()) == 0 && strcmp(next_node.my_value, value.c_str()) == 0 && strcmp(next_node.my_context, context.c_str()) == 0)
            {
                BinaryFile::Offset new_reused = node.next;
                node.next = next_node.next;
                my_file.write(node, current_node);
                num_of_erase++;
                
                my_file.read(reused_location, my_location(num_of_buckets));
                next_node.next = reused_location;
                my_file.write(next_node, new_reused);
                my_file.write(new_reused, my_location(num_of_buckets));
            }
            else current_node = node.next;
        }
        else break;
    }
    return num_of_erase;
}


