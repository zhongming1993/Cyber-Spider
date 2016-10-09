//
//  IntelWeb.cpp
//  p4
//
//  Created by MingZhong on 3/8/16.
//  Copyright © 2016 MingZhong. All rights reserved.
//

#include "IntelWeb.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <set>
#include <queue>

bool operator<(const InteractionTuple tupleA, const InteractionTuple tupleB)
{
    if (tupleA.context < tupleB.context)
        return true;
    else if (tupleA.context > tupleB.context)
        return false;
    else
    {
        if (tupleA.from < tupleB.from)
            return true;
        else if (tupleA.from > tupleB.from)
            return false;
        else
        {
            if (tupleA.to < tupleB.to)
                return true;
            else return false;
        }
    }
}

/*
void IntelWeb::print()
{
    cout << " source to target hash table: " << endl;
    source_to_target.printAll();
    cout << "******************************************************" << endl;
    cout << " target to source hash table: " << endl;
    target_to_source.printAll();
    cout << "******************************************************" << endl;
    cout << " prevalence hash table " << endl;
    prevalence_hash_table.printAll();    
}
*/

IntelWeb::IntelWeb()
{
    source_to_target_filename_postfix = string1;
    target_to_source_filename_postfix = string2;
    prevalence_filename_postfix = string3;
}

IntelWeb::~IntelWeb()
{
    close();
}

bool IntelWeb::createNew(const std::string &filePrefix, unsigned int maxDataItems)
{
    int num_of_buckets = maxDataItems * loading_factor;
    if (source_to_target.createNew(filePrefix+source_to_target_filename_postfix, num_of_buckets)
     && target_to_source.createNew(filePrefix+target_to_source_filename_postfix, num_of_buckets)
    && prevalence_hash_table.createNew(filePrefix+prevalence_filename_postfix, num_of_buckets))
        return true;
    else
    {
        close();
        return false;
    }
}

bool IntelWeb::openExisting(const std::string &filePrefix)
{
    if (source_to_target.openExisting(filePrefix+source_to_target_filename_postfix)
        && target_to_source.openExisting(filePrefix+target_to_source_filename_postfix)
        && prevalence_hash_table.openExisting(filePrefix+prevalence_filename_postfix))
        return true;
    else
    {
        close();
        return false;
    }
}

void IntelWeb::close()
{
    source_to_target.close();
    target_to_source.close();
    prevalence_hash_table.close();
}

bool IntelWeb::ingest(const std::string &telemetryFile)
{
    ifstream input_file(telemetryFile);
    if (!input_file)
    {
        cerr << "Cannot open log file!" << endl;
        return false;
    }
    std::string line;
    std::unordered_map<std::string, int> prevalence_map;
    std::unordered_map<std::string, int>::iterator it;
    while (getline(input_file, line))
    {
        istringstream ssline(line);
        // m491       fd.exe       bwk.exe
        // machine    source       target
        string machine;
        string source;
        string target;
        if (!(ssline >> machine >> source >> target))
        {
            cerr << "Ignoring badly-formatted input line: " << line << endl;
            continue;
        }
        // source(key), target(value), machine(context)
        source_to_target.insert(source, target, machine);
        // target(key), source(value), machine(context)
        target_to_source.insert(target, source, machine);
        // store each entity's prevalence to prevalence_hash_table
        it = prevalence_map.find(source);
        // this entity already exists in the prevalence_map
        if (it != prevalence_map.end())
            prevalence_map[source] = it -> second + 1;
        else
            prevalence_map[source] = 1;
        it = prevalence_map.find(target);
        // this entity already exists in the prevalence_map
        if (it != prevalence_map.end())
            prevalence_map[target] = it -> second + 1;
        else
            prevalence_map[target] = 1;
    }
    // entity(key), prevalence(value), ""(context)
    for (it = prevalence_map.begin(); it != prevalence_map.end(); it++)
    {
        DiskMultiMap::Iterator temp = prevalence_hash_table.search(it -> first);
        if (!temp.isValid())
            prevalence_hash_table.insert(it -> first, to_string(it->second), "");
        else
        {
            int new_prevalence = stoi((*temp).value) + (it -> second);
            prevalence_hash_table.erase((*temp).key, (*temp).value, (*temp).context);
            prevalence_hash_table.insert(it -> first, to_string(new_prevalence), "");
        }
    }
    return true;
}

int IntelWeb::prevalence(std::string entity)
{
    DiskMultiMap::Iterator it = prevalence_hash_table.search(entity);
    // if the entity does not exist in the log file
    if (!it.isValid())
        return 0;
    else
    {
        string prevalence_number = (*it).value;
        return std::stoi(prevalence_number);
    }
}


unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators, unsigned int minPrevalenceToBeGood,
                   std::vector<std::string>& badEntitiesFound, std::vector<InteractionTuple>& interactions)
{
    badEntitiesFound.clear();
    interactions.clear();
    std::set<InteractionTuple> bad_interaction;
    std::set<std::string> bad_entities_confirmed;
    // a queue stores malicious entities to be solved later.
    queue<std::string> bad_entities_to_solve;
    for (int i = 0; i < indicators.size(); i++)
    {
        // the ith element in the indicator appears in the log file
        DiskMultiMap::Iterator it = prevalence_hash_table.search(indicators[i]);
        if (it.isValid())
        {
            //cerr << "insert known bad into bad_entities to solve: " << indicators[i] << endl;
            bad_entities_to_solve.push(indicators[i]);
            bad_entities_confirmed.insert(indicators[i]);
        }
    }
    while (!bad_entities_to_solve.empty())
    {
        std::string current_bad_entity = bad_entities_to_solve.front();
        //cerr << "known malicious entity pop from bad_entities_to_solve: " << current_bad_entity << endl;
        bad_entities_to_solve.pop();
        DiskMultiMap::Iterator is = source_to_target.search(current_bad_entity);
        while (is.isValid())
        {
            std::string potential_bad_entity = (*is).value;
            //cerr << "source to target: related entity: " << potential_bad_entity << endl;
            InteractionTuple my_interaction((*is).key, (*is).value, (*is).context);
            bad_interaction.insert(my_interaction);
            // if the prevamence is smaller than the limit value, this entity is bad
            if (prevalence(potential_bad_entity) < minPrevalenceToBeGood)
            {
                //cerr << "this related entity is bad" << endl;
                // check is this entity has already appeared in bad_entities_confirmed
                // if not, insert this entity both to bad_entities_confirmed and bad_entities_to_solve
                if (bad_entities_confirmed.find(potential_bad_entity) == bad_entities_confirmed.end())
                {
                    //cerr << "this related bad entity does not appear before, should be inserted to bad_entities" << endl;
                    bad_entities_to_solve.push(potential_bad_entity);
                    bad_entities_confirmed.insert(potential_bad_entity);
                }
            }
            ++is;
        }
        DiskMultiMap::Iterator it = target_to_source.search(current_bad_entity);
        while (it.isValid())
        {
            std::string potential_bad_entity = (*it).value;
            //cerr << "target to source: related entity: " << potential_bad_entity << endl;
            InteractionTuple my_interaction((*it).value, (*it).key, (*it).context);
            bad_interaction.insert(my_interaction);
            if (prevalence(potential_bad_entity) < minPrevalenceToBeGood)
            {
                //cerr << "this related entity is bad" << endl;
                if (bad_entities_confirmed.find(potential_bad_entity) == bad_entities_confirmed.end())
                {
                    //cerr << "this related bad entity does not appear before, should be inserted to bad_entities" << endl;
                    bad_entities_to_solve.push(potential_bad_entity);
                    bad_entities_confirmed.insert(potential_bad_entity);
                }
            }
            ++it;
        }
    }
    // assign the result to the parameters
    //cerr << "badEntitiesFound:  " << endl;
    for (set<std::string>::iterator it = bad_entities_confirmed.begin(); it != bad_entities_confirmed.end(); it++)
    {
        //cerr << (*it) << endl;
        badEntitiesFound.push_back(*it);
    }
    //cerr << "badInteractionsFound:  " << endl;
    for (set<InteractionTuple>::iterator is = bad_interaction.begin(); is != bad_interaction.end(); is++)
    {
        //cerr << (*is).from << "   " << (*is).to << "   " << (*is).context << endl;
        interactions.push_back(*is);
    }
    return badEntitiesFound.size();
}

void IntelWeb::update_prevalence(std::string entity, int num)
{
    DiskMultiMap::Iterator it = prevalence_hash_table.search(entity);
    if (it.isValid())
    {
        int new_prevalence = (stoi((*it).value) - num);
        prevalence_hash_table.erase((*it).key, (*it).value, (*it).context);
        prevalence_hash_table.insert((*it).key, to_string(new_prevalence), (*it).context);
    }
    else
        cerr << "updating the prevalence of a nonexsiting entity" << endl;
}

bool IntelWeb::purge(const std::string& entity)
{
    std::string source;
    std::string target;
    std::string machine;
    bool is_purged = 0;
    DiskMultiMap::Iterator is = source_to_target.search(entity);
    while (is.isValid())
    {
        source = (*is).key;
        target = (*is).value;
        machine = (*is).context;
        ++is;
        while (is.isValid() && (*is).key == source && (*is).value == target && (*is).context == machine)
            ++is;
        is_purged = 1;
        int num_of_node_erased = source_to_target.erase(source, target, machine);
        DiskMultiMap::Iterator it= target_to_source.search(target);
        if (it.isValid())
        {
            target_to_source.erase(target, source, machine);
            update_prevalence(source, num_of_node_erased);
            update_prevalence(target, num_of_node_erased);
        }
    }
    
    DiskMultiMap::Iterator im = target_to_source.search(entity);
    while (im.isValid())
    {
        source = (*im).value;
        target = (*im).key;
        machine = (*im).context;
        ++im;
        while (im.isValid() && (*im).value == source && (*im).key == target && (*im).context == machine)
            ++im;
        is_purged = 1;
        int num_of_node_erased = target_to_source.erase(target, source, machine);
        DiskMultiMap::Iterator in = source_to_target.search(source);
        if (in.isValid())
        {
            source_to_target.erase(source, target, machine);
            update_prevalence(source, num_of_node_erased);
            update_prevalence(target, num_of_node_erased);
        }
    }
    return is_purged;
}