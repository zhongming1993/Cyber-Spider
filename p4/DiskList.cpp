//
//  main.cpp
//  p4_warmip
//
//  Created by MingZhong on 3/6/16.
//  Copyright © 2016 MingZhong. All rights reserved.
//

#include <iostream>
#include "BinaryFile.h"

// List node fo the binary file c_string + next;
struct DiskNode
{
    char my_char[256];
    BinaryFile::Offset next; // instead of a DiskNode *
    bool valid;
};

class DiskList
{
public:
    DiskList(const std::string& filename);
    bool push_front(const char* data);
    bool remove(const char* data);
    void printAll();
private:
    BinaryFile::Offset find_available_location();
    BinaryFile my_file;
    int nums;
    BinaryFile::Offset next_available_memory;
};

BinaryFile::Offset DiskList::find_available_location()
{
    for (BinaryFile::Offset i = 4; i < next_available_memory; i = i + sizeof(DiskNode))
    {
        DiskNode node;
        my_file.read(node, i);
        if (node.valid == 0)
        {
            return i;
        }
    }
    return next_available_memory;
}

DiskList::DiskList(const std::string& filename)
{
    nums = 0;
    next_available_memory = 4;
    cout << my_file.isOpen() << endl;
    bool success = my_file.createNew(filename);
    if (!success)
        cout << "Error! Unable to create myfile";
    // set head pointer to nullptr;
    BinaryFile::Offset offsetOfFirstNode = 0;
    my_file.write(offsetOfFirstNode, 0);
}

bool DiskList::push_front(const char* data)
{
    //C string has at least 256 non-zero-byte characters
    if (strlen(data) >= 256)
        return false;
    //C string has < 256 non-zero-byte characters
    else
    {
        nums++;
        // insert the node at the front
        DiskNode node;
        strcpy(node.my_char, data);
        BinaryFile::Offset head;
        if (my_file.read(head, 0))
            node.next = head;
        else cout << " head read error!" << endl;
        node.valid = 1;
        // update the head pointer
        BinaryFile::Offset location = find_available_location();
        if (!my_file.write(location, 0))
            cout << " head update error!" << endl;
        if (!my_file.write(node, location))
        cout << "Error writing a new node to the file\n";
        else
        {
            if (location == next_available_memory)
                next_available_memory = next_available_memory + sizeof(DiskNode);
        }
        return true;
    }
}

bool DiskList::remove(const char* data)
{
    bool is_removed = 0;
    BinaryFile::Offset current_node;
    BinaryFile::Offset head;
    if (!my_file.read(head, 0))
        cout << " head read error!" << endl;
    // check if the first node should be removed.
    while (head != 0)
    {
        DiskNode node;
        my_file.read(node, head);
        // if this node need to be removed.
        if (strcmp(node.my_char, data) == 0)
        {
            // update the node we want to delete as invalid.
            node.valid = 0;
            my_file.write(node, head);
            head = node.next;
            // update head.
            my_file.write(head, 0);
            is_removed = 1;
            nums--;
        }
        else break;
    }
    if (my_file.read(head, 0))
        current_node = head;
    else cout << " head read error!" << endl;
    while (current_node!=0)
    {
        DiskNode node;
        my_file.read(node, current_node);
        if (node.next!=0)
        {
            DiskNode next_node;
            my_file.read(next_node, node.next);
            if (strcmp(next_node.my_char, data) == 0)
            {
                next_node.valid = 0;
                my_file.write(next_node, node.next);
                node.next = next_node.next;
                my_file.write(node, current_node);
                is_removed = 1;
                nums--;
            }
            else current_node = node.next;
        }
        else break;
    }
    return is_removed;
}

void DiskList::printAll()
{
    // set the current_node to head
    BinaryFile::Offset current_node;
    BinaryFile::Offset head;
    if (my_file.read(head, 0))
        current_node = head;
    else cout << " head read error!" << endl;
    while (current_node!=0)
    {
        //cerr << current_node << "    ";
        DiskNode node;
        if (my_file.read(node, current_node))
        {
            for (int i = 0; i < strlen(node.my_char); i++)
                cout << node.my_char[i];
            // print the c string of this node.
            cout << endl;
        }
        else cout << "read node error in printing" << endl;
        current_node = node.next;
    }
}


int main()
{
        DiskList x("mylist.dat");
        x.push_front("Fred");
        x.push_front("Lucy");
        x.push_front("Ethel");
        x.push_front("Ethel");
        x.push_front("Lucy");
        x.push_front("Fred");
        x.push_front("Ethel");
        x.push_front("Ricky");
        x.push_front("Lucy");
        x.remove("Lucy");
        x.push_front("Fred");
        x.push_front("Ricky");
        x.push_front("Ricky1");
        x.push_front("Ricky2");
        x.push_front("Ricky3");
        x.printAll();  // writes, one per line
        // Ricky  Fred  Ricky  Ethel  Fred  Ethel  Ethel  Fred
}
