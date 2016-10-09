//
//  main.cpp
//  p4
//
//  Created by MingZhong on 3/6/16.
//  Copyright © 2016 MingZhong. All rights reserved.
//

#include "DiskMultiMap.h"
#include "IntelWeb.h"

void f()
{
    DiskMultiMap d;
    d.createNew("test", 1000);
    d.insert("hmm.exe","pfft.exe","m52902");
    d.insert("hmm.exe","pfft.exe","m52902");
    d.insert("hmm.exe","pfft.exe","m10001");
    d.insert("blah.exx","bletch.exe","m0003");
    // line 1
    if (d.erase("hmm.exe", "pfft.exe", "m52902") == 2)
        cout << "Just erased 2 items from the table!\n";
    // line 2
    if (d.erase("hmm.exe", "pfft.exe", "m10001") > 0)
        cout << "Just erased at least 1 item from the table!\n";
    // line 3
    if (d.erase("blah.exe", "bletch.exe", "m66666") == 0)
        cout << "I didn't erase this item cause it wasn't there\n";
    //d.printAll();
    d.insert("hmm.exe","pfft.exe","m1");
    d.insert("hmm.exe","pfft.exe","m2");
    d.insert("kij.exe","pfft.exe","m3");
    d.insert("hjkkkkk","pfft.exe","m4");
    //d.printAll();
}
int main()
{
    //f();
   // DiskMultiMap b;
    //b.openExisting("test.txt");
    //b.printAll();
    IntelWeb x;
    x.createNew("north-amarican-data", 200);
    x.ingest("test.txt");
    cout << "**************************************************" << endl;
    //x.print();
    vector<std::string> known_bad;
    vector<std::string> found_bad;
    vector<InteractionTuple> bad_interactions;
    known_bad.push_back("a.exe");
    known_bad.push_back("c.exe");
    int k = x.crawl(known_bad, 4, found_bad, bad_interactions);
    cout << "number of bad entities: " <<  k << endl;
    x.purge("b.exe");
    //x.print();
}
