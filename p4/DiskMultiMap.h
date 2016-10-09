//
//  Header.h
//  p4
//
//  Created by MingZhong on 3/6/16.
//  Copyright © 2016 MingZhong. All rights reserved.
//

#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#include <string>
#include "MultiMapTuple.h"
#include "BinaryFile.h"

class DiskMultiMap
{
public:
    
    class Iterator
    {
    public:
        Iterator();
        Iterator(DiskMultiMap* x, BinaryFile::Offset position = 0);
        ~Iterator();
        bool isValid() const;
        Iterator& operator++();
        MultiMapTuple operator*();
        
    private:
        // a pointer to the DiskMultiMap that this iterator is defined in.
        DiskMultiMap * parent;
        BinaryFile::Offset my_position;
    };
    
    DiskMultiMap();
    ~DiskMultiMap();
    bool createNew(const std::string& filename, unsigned int numBuckets);
    bool openExisting(const std::string& filename);
    void close();
    bool insert(const std::string& key, const std::string& value, const std::string& context);
    Iterator search(const std::string& key);
    int erase(const std::string& key, const std::string& value, const std::string& context);
    
private:
    // find any location that can be reused, if not, return next_available_memory.
    BinaryFile::Offset find_available_location();
    // initalize the buckets and next_available_memory.
    void initialize_hash_table();
    // hash function that hash this string into an unsigned integer
    unsigned int hash_me(const std::string & str);
    // return the location where the the current bucket is stored.
    // i is the index of the bucket when i is between 0 ~ num_of_buckets-1
    // when i = num_of_bucket, it returns the location of reused_location
    BinaryFile::Offset my_location(int i);
    
    // bucket struct for hashtable
    struct bucket
    {
        BinaryFile::Offset head;
        int bucket_index;
        bucket(){}
        bucket(int i):head(0), bucket_index(i){}
    };
    
    struct DiskNode
    {
        char my_key[121];
        char my_value[121];
        char my_context[121];
        // instead of a DiskNode *
        BinaryFile::Offset next;
        bool valid;
        // constructor for Disknode to check the result.
        DiskNode(){}
        // constructor for the node to be insert
        DiskNode(const char* key, const char* value, const char* context, BinaryFile::Offset next_position, bool is_valid)
        {
            strcpy(my_key, key);
            strcpy(my_value, value);
            strcpy(my_context, context);
            next = next_position;
            valid = is_valid;
        }
    };
    
    // in my_file, the first element is num_of_buckets, the second element is next_available_memory
    // then are buckets, then the reused_location, then the nodes.
    BinaryFile my_file;
    unsigned int num_of_buckets;
    //string current_file_name;
    BinaryFile::Offset next_available_memory;
};

#endif // DISKMULTIMAP_H_
