//
//  Header.h
//  p4
//
//  Created by MingZhong on 3/6/16.
//  Copyright © 2016 MingZhong. All rights reserved.
//

#ifndef INTELWEB_H_
#define INTELWEB_H_

#include "InteractionTuple.h"
#include "DiskMultiMap.h"
#include <string>
#include <vector>

const std::string string1 = "inorder";
const std::string string2 = "reverse";
const std::string string3 = "prevalence";
const double loading_factor = 0.5;

class IntelWeb
{
public:
    IntelWeb();
    ~IntelWeb();
    bool createNew(const std::string& filePrefix, unsigned int maxDataItems);
    bool openExisting(const std::string& filePrefix);
    void close();
    bool ingest(const std::string& telemetryFile);
    unsigned int crawl(const std::vector<std::string>& indicators,
                       unsigned int minPrevalenceToBeGood,
                       std::vector<std::string>& badEntitiesFound,
                       std::vector<InteractionTuple>& interactions
                       );
    bool purge(const std::string& entity);
    
private:
    int prevalence(std::string entity);
    void update_prevalence(std::string entity, int num = 1);
    DiskMultiMap source_to_target;
    DiskMultiMap target_to_source;
    DiskMultiMap prevalence_hash_table;
    std::string source_to_target_filename_postfix;
    std::string target_to_source_filename_postfix;
    std::string prevalence_filename_postfix;
};

#endif // INTELWEB_H_

