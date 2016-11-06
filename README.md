# Cyber-Spider
In this project, I built a spider that crawls the database to identify suspicious entities from a list of know threat indicators. Assume every single government-owned computer monitors and collects exhaustive log data on its activity like what's stored in malicious.txt. Given the known malicious entities, this program crawls through the log data and find potential malicious entities. As the amount of log data is very large in real world, all of the storage data structure are implemented at disk level in this project. In order to make crawling more efficient, log data are store in hash tables.

1. The first part of this project is to create new data structure called DiskNode, which can be used to form a disk-basked linked list(BinaryFile.h & DiskList.cpp)

2. The second part of this project is to implemented a disk-based, hash-table-based multimap, to simulate the real world situation where all data can not fit into memory.(DiskMultiMap.h & DiskMultiMap.cpp)

3. The third part of this project is to build a top level spider, which is able to digest the log data into hash table, crawl through the hash table to find malicious entities.(IntelWeb.h & IntelWeb.cpp)
