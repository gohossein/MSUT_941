#include <iostream>
#include <stdio.h>
#include <boost/algorithm/string.hpp> // for split function
#include <vector>
#include <fstream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <map>
#include <boost/program_options.hpp>

namespace po = boost::program_options;






void show_help(const po::options_description& desc, const std::string& topic = "")
{
    std::cout << desc << '\n';
    if (topic != "") {
        std::cout << "You asked for help on: " << topic << '\n';
    }
    exit( EXIT_SUCCESS );
}

void parse_program_options(const int argc, const char *const argv[], std::string& infile, std::string& mapfilename, int& count_bins_number, int& verbose)
{
    po::options_description desc("Usage");
    desc.add_options()
        (
            "help,h",
            po::value< std::string >()
                ->implicit_value("")
                ->notifier(
                    [&desc](const std::string& topic) {
                        show_help(desc, topic);
                    }
                ),
            "Show help. If given, show help on the specified topic."
        )
        ("verbose,v", po::value<int>(&verbose)->default_value(0),"set 1 for verbosity, defualt = 0")
        ("count_bins_number,c", po::value<int>(&count_bins_number)->default_value(10),"set number of count bins")
    	("infile,i", po::value<std::string>(&infile)->required(), "input file name")
    	("mapfile,m", po::value<std::string>(&mapfilename)->required(), "map table file name")
    ;

    if (argc == 1) {
        show_help(desc); // does not return
    }

    

    try {
    	po::variables_map args;
        po::store(
            po::parse_command_line(argc, argv, desc),
            args
        );
        po::notify(args);
    }
    catch (po::error const& e) {
        std::cerr << e.what() << '\n';
        exit( EXIT_FAILURE );
    }
    
    return;
}


int main(int argc, char *argv[])
{
    time_t start, end, start_map, end_map, start_train, end_train, start_test, end_test, start_map_write, end_map_write;
    time(&start);
    std::string inputfilename, outputfilename, mapfilename, mapfilenametest, testfilename, outputtestfilename, line, map_name;
    int verbose, count_bins_number, col_index, data_target, Index_Counter, Max_Count;
    parse_program_options(argc, argv, inputfilename, mapfilename, count_bins_number, verbose);
    std::vector<int> bins_count; // number of clicks in one bins
    std::vector<int> Count_Data; // all click data
    std::vector<int>::iterator it;

/* ------------------------------------------------------------------------------------------

	read file line by line add each value to map


------------------------------------------------------------------------------------------ */
	
	time(&start_map);
	std::ifstream csv_file;
	csv_file.open(inputfilename.c_str());
    if(!csv_file.is_open())
    {
        std::cout<<"input file not found!"<<std::endl;
        return 0;
    }
	csv_file.clear(); //Clear State
    csv_file.seekg(0,std::ios::beg);
	  	
    // read headers
    //getline(csv_file,line);
    //boost::split( header_col_names, line, boost::is_any_of(","), boost::token_compress_on );
    it = Count_Data.begin();
    while(std::getline(csv_file, line))
    {
        it = Count_Data.insert(it,atoi(line.c_str()));
    
    }
    Max_Count = 0;
    for (auto val : Count_Data)
    {
        if ( val > Max_Count)
            Max_Count = val;
    }
    if(count_bins_number > Count_Data.size())
    {
        count_bins_number = Count_Data.size();
    }
    it = bins_count.begin();
    for (int i=0; i<count_bins_number;++i)
    {
        it = bins_count.insert(it,0);

    }
    float temp_count;

    for (auto val : Count_Data)
    {
        if ( val == Max_Count)
        {
            bins_count[count_bins_number-1]++;
        }else {
            temp_count = val * count_bins_number; 
            bins_count[temp_count / Max_Count] ++;
        }
        
        // std::cout << temp_count << std::endl;
    }

    
  	time(&end_map);

/* ------------------------------------------------------------------------------------------
	write map file
   ------------------------------------------------------------------------------------------ */

	time(&start_map_write);
    std::ofstream data_map_feature;
	data_map_feature.open (mapfilename);//"Transform_Data_Sample_Test/data_map_feature.csv");
	if(!data_map_feature.is_open())
    {
        std::cout<<"could not create or open map file!"<<std::endl;
        return 0;
    }

	data_map_feature << "Bin Number" << "," << "Bin Counts" << std::endl;
	 for (int i=0; i<count_bins_number;++i)
    {
        data_map_feature << i << "," << bins_count[i] << std::endl;

    }
    
	
    data_map_feature.close();
    time(&end_map_write);

    std::cout << " Data Size: " << Count_Data.size() << std::endl;
    std::cout << "Max_Count : " << Max_Count << std::endl;
    std::cout << "Number of Bins: " << count_bins_number << std::endl;

	time(&end_test);
	time(&end);
	std::cout<<"Total Time: "<<difftime(end,start)<<std::endl;
	
	return 0;

}



//compile: g++ -std=c++11 -lboost_program_options -o Click_Count_Histogram Click_Count_Histogram.cpp
// ./Click_Count_Histogram -i C14_Click.csv -v 1 -c 10 -m C14_Hist.csv





