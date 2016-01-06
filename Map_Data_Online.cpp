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


// struct click for map
struct click {
int click_no;
int non_click_no;
};




void show_help(const po::options_description& desc, const std::string& topic = "")
{
    std::cout << desc << '\n';
    if (topic != "") {
        std::cout << "You asked for help on: " << topic << '\n';
    }
    exit( EXIT_SUCCESS );
}

void parse_program_options(const int argc, const char *const argv[], std::string& infile, std::string& mapfilename, std::string& outfile, std::string& testfilename, std::string& outputtestfilename, std::vector<std::string>& ignorecols, int& click_ratio, int& verbose)
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
        ("clickratio,c", po::value<int>(&click_ratio)->default_value(0),"set 1 for adding click ratio for each column, defualt = 0")
    	("infile,i", po::value<std::string>(&infile)->required(), "input file name")
    	("outfile,o", po::value<std::string>(&outfile)->required(), "output file name")
    	("mapfile,m", po::value<std::string>(&mapfilename)->required(), "map table file name")
    	("testfile,e", po::value<std::string>(&testfilename), "test file name")
    	("outtestfile,t", po::value<std::string>(&outputtestfilename), "test output file name")
    	("skipcols,s", po::value<std::vector<std::string>>(&ignorecols)->multitoken(), "list of columns to be skiped, use space  for item seperator")
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
    std::string inputfilename, outputfilename, mapfilename, testfilename, outputtestfilename, line, map_name;
    std::vector<std::string> list_skiped_cols, data_row, header_col_names, header_col_names_two;
    int verbose, click_ratio, col_index, data_target;
    std::map<std::string, click>* data_click = new std::map<std::string, click>;
    click click_feature{};
    parse_program_options(argc, argv, inputfilename, mapfilename, outputfilename, testfilename, outputtestfilename, list_skiped_cols, click_ratio, verbose);




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
    getline(csv_file,line);
    boost::split( header_col_names, line, boost::is_any_of(","), boost::token_compress_on );
    while(!csv_file.eof())
    {
    	getline(csv_file,line);
    	if( csv_file.eof() ) break; // for last empty line!
    	boost::split( data_row, line, boost::is_any_of(","), boost::token_compress_on ); // SplitVec == { "hello abc","ABC","aBc goodbye" }
    	data_target = atoi(data_row[0].c_str()); // add click to target column
    	data_row.erase(data_row.begin()); // remove click value
    	col_index = 0;
    	for (auto & element : data_row)
    	{
    		col_index ++;	
    		if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), header_col_names[col_index]) == list_skiped_cols.end())
    		{
    			map_name = header_col_names[col_index]+"_"+element;
                if ( !data_click->count(map_name) ) // !data_click->count(map_name) & (map_name != (header_col_names[col_index]+"_"))
                {
                    // not found
                    click_feature.click_no = data_target;
                    click_feature.non_click_no = 1 - data_target;
                    (*data_click)[map_name] = click_feature;  
                }else{

                    (*data_click)[map_name].click_no = (*data_click)[map_name].click_no + data_target;
                    (*data_click)[map_name].non_click_no = (*data_click)[map_name].non_click_no + 1 - data_target;  
   				}
                map_name.clear();
    		}

    	}
    	data_row.clear();
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
    if(!click_ratio){
    	data_map_feature << "Feature_Name: F_N" << "," << "Feature_#Clicks: F_C" << "," << "Feature_#Non_Clicks: F_N_C" << std::endl;
		data_map_feature << "F_N" << "," << "F_C" << "," << "F_N_C" << std::endl;
    	for(auto elem : (*data_click))
	    {
	        data_map_feature << elem.first << "," << elem.second.click_no << "," << elem.second.non_click_no << std::endl;
	    }
    }else{
    	data_map_feature << "Feature_Name: F_N" << "," << "Feature_#Clicks: F_C" << "," << "Feature_#Non_Clicks: F_N_C" << "," << "Feature_Click_Ratio: F_C_R" << std::endl;
		data_map_feature << "F_N" << "," << "F_C" << "," << "F_N_C" << "," << "F_C_R" << std::endl;
		//write map to file
	    for(auto elem : (*data_click))
	    {
	        data_map_feature << elem.first << "," << elem.second.click_no << "," << elem.second.non_click_no << "," << (*data_click)[map_name].click_no / float(((*data_click)[map_name].click_no+(*data_click)[map_name].non_click_no)) << std::endl;
	    }
    }
	
    data_map_feature.close();
    time(&end_map_write);


/* ------------------------------------------------------------------------------------------
	create output file from map and input file
   ------------------------------------------------------------------------------------------ */

	time(&start_train);
	std::ofstream train_output;
    train_output.open(outputfilename);//"Transform_Data_Sample_Test/demo_map_final.txt");
	if(!train_output.is_open())
    {
        std::cout<<"could not create or open train output file!"<<std::endl;
        return 0;
    }

	csv_file.clear(); //Clear State
    csv_file.seekg(0,std::ios::beg);
	  	
    // read headers and write new header for output file
    getline(csv_file,line);
    boost::split( header_col_names_two, line, boost::is_any_of(","), boost::token_compress_on );
    train_output << header_col_names_two[0];
    header_col_names_two.erase(header_col_names_two.begin());
    
    if(!click_ratio)
    {
		for (auto & element : header_col_names_two)
	    {
	    	if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), element) == list_skiped_cols.end())
	    	{
	    		train_output << "," << element + "_C" << "," << element + "_N_C";
	    	}
	    }
	    train_output << std::endl;

	    while(!csv_file.eof())
	    {
	    	getline(csv_file,line);
	    	if( csv_file.eof() ) break; // for last empty line!
	    	boost::split( data_row, line, boost::is_any_of(","), boost::token_compress_on ); // SplitVec == { "hello abc","ABC","aBc goodbye" }
	    	data_target = atoi(data_row[0].c_str()); // add click to target column
	    	data_row.erase(data_row.begin()); // remove click value
	    	col_index = 0;
	    	train_output << data_target;
	    	for (auto & element : data_row)
	    	{	
	    		if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), header_col_names_two[col_index]) == list_skiped_cols.end())
	    		{
	    			map_name = header_col_names_two[col_index]+"_"+element;    
					train_output << "," << (*data_click)[map_name].click_no << "," << (*data_click)[map_name].non_click_no;
	                
	   			}
	            map_name.clear();
	            col_index ++;
	    	}
	    	train_output << std::endl;
	   		data_row.clear();
	    }
    }else{
    	for (auto & element : header_col_names_two)
	    {
	    	if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), element) == list_skiped_cols.end())
	    	{
	    		train_output << "," << element + "_C" << "," << element + "_N_C" << element + "_C_R";
	    	}
	    }
	    train_output << std::endl;

	    while(!csv_file.eof())
	    {
	    	getline(csv_file,line);
	    	if( csv_file.eof() ) break; // for last empty line!
	    	boost::split( data_row, line, boost::is_any_of(","), boost::token_compress_on ); // SplitVec == { "hello abc","ABC","aBc goodbye" }
	    	data_target = atoi(data_row[0].c_str()); // add click to target column
	    	data_row.erase(data_row.begin()); // remove click value
	    	col_index = 0;
	    	train_output << data_target;
	    	for (auto & element : data_row)
	    	{	
	    		if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), header_col_names_two[col_index]) == list_skiped_cols.end())
	    		{
	    			map_name = header_col_names_two[col_index]+"_"+element;    
					train_output << "," << (*data_click)[map_name].click_no << "," << (*data_click)[map_name].non_click_no << "," << (*data_click)[map_name].click_no / float(((*data_click)[map_name].click_no+(*data_click)[map_name].non_click_no)); 
	                
	   			}
	            map_name.clear();
	            col_index ++;
	    	}
	    	train_output << std::endl;
	   		data_row.clear();
	    }
    }

    
    train_output.close();
    csv_file.close();
    time(&end_train);

/* ------------------------------------------------------------------------------------------
	create test output file from map and test input file
   ------------------------------------------------------------------------------------------ */
	
	time(&start_test);
	if(testfilename!="")
	{
		csv_file.open(testfilename.c_str());
	    if(!csv_file.is_open())
	    {
	        std::cout<<"test input file not found!"<<std::endl;
	        return 0;
	    }
		train_output.open(outputtestfilename);//"Transform_Data_Sample_Test/demo_map_final.txt");
		if(!train_output.is_open())
	    {
	        std::cout<<"could not create or open test output file!"<<std::endl;
	        return 0;
	    }

		csv_file.clear(); //Clear State
	    csv_file.seekg(0,std::ios::beg);
		  	
	    // read headers and write new header for output file
	    getline(csv_file,line);
	    boost::split( header_col_names_two, line, boost::is_any_of(","), boost::token_compress_on );
	    train_output << header_col_names_two[0];
	    header_col_names_two.erase(header_col_names_two.begin());
	    
	    if(!click_ratio)
	    {
	    	for (auto & element : header_col_names_two)
		    {
		    	if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), element) == list_skiped_cols.end())
		    	{
		    		train_output << "," << element + "_C" << "," << element + "_N_C";
		    	}
		    }
		    train_output << std::endl;
		    while(!csv_file.eof())
		    {
		    	getline(csv_file,line);
		    	if( csv_file.eof() ) break; // for last empty line!
		    	boost::split( data_row, line, boost::is_any_of(","), boost::token_compress_on ); // SplitVec == { "hello abc","ABC","aBc goodbye" }
		    	data_target = atoi(data_row[0].c_str()); // add click to target column
		    	data_row.erase(data_row.begin()); // remove click value
		    	col_index = 0;
		    	train_output << data_target;
		    	for (auto & element : data_row)
		    	{	
		    		if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), header_col_names_two[col_index]) == list_skiped_cols.end())
		    		{
		    			map_name = header_col_names_two[col_index]+"_"+element;
		    			if ( !data_click->count(map_name) ) // !data_click->count(map_name) & (map_name != (header_col_names[col_index]+"_"))
		                {
		                    // not found
		                    //train_output << "," << data_target << "," << 1 - data_target;
		                	train_output << "," << 0 << "," << 0;
		                }else{

		                    train_output << "," << (*data_click)[map_name].click_no << "," << (*data_click)[map_name].non_click_no; 
		   				}    
		   			}
		            map_name.clear();
		            col_index ++;
		    	}
		    	train_output << std::endl;
		   		data_row.clear();
		    }
	    }else{
	    	for (auto & element : header_col_names_two)
		    {
		    	if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), element) == list_skiped_cols.end())
		    	{
		    		train_output << "," << element + "_C" << "," << element + "_N_C" << element + "_C_R";
		    	}
		    }
		    train_output << std::endl;
		    while(!csv_file.eof())
		    {
		    	getline(csv_file,line);
		    	if( csv_file.eof() ) break; // for last empty line!
		    	boost::split( data_row, line, boost::is_any_of(","), boost::token_compress_on ); 
		    	data_target = atoi(data_row[0].c_str()); // add click to target column
		    	data_row.erase(data_row.begin()); // remove click value
		    	col_index = 0;
		    	train_output << data_target;
		    	for (auto & element : data_row)
		    	{	
		    		if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), header_col_names_two[col_index]) == list_skiped_cols.end())
		    		{
		    			map_name = header_col_names_two[col_index]+"_"+element;
		    			if ( !data_click->count(map_name) ) 
		                {
		                    // not found
		                    //train_output << "," << data_target << "," << 1 - data_target << data_target; 
		                    train_output << "," << 0 << "," << 0 << "," << 0; 
		                }else{

		                    train_output << "," << (*data_click)[map_name].click_no << "," << (*data_click)[map_name].non_click_no << "," << (*data_click)[map_name].click_no / float(((*data_click)[map_name].click_no+(*data_click)[map_name].non_click_no)); 
		   				}
		   			}
		            map_name.clear();
		            col_index ++;
		    	}
		    	train_output << std::endl;
		   		data_row.clear();
		    }
	    }
	    train_output.close();
	    csv_file.close();
	}
	time(&end_test);
	time(&end);
	std::cout<<"Total Time: "<<difftime(end,start)<<std::endl;
	if(verbose)
	{
		std::cout<<"Mapping Time: "<<difftime(end_map,start_map)<<std::endl;
		std::cout<<"Writing Map File Time: "<<difftime(end_map_write,start_map_write)<<std::endl;
		std::cout<<"Writing Train File Time: "<<difftime(end_train,start_train)<<std::endl;
		std::cout<<"Writing Test Time: "<<difftime(end_test,start_test)<<std::endl;
	}
	return 0;

}



//compile: g++ -std=c++11 -lboost_program_options -o data_click_table_online Map_Data_Online.cpp
//run: ./data_click_table_online -s app_id site_id -o t_out.txt -m mapt.txt -v 1 -i demo_50.txt
// ./data_click_table_online -s site_id device_ip device_model device_id -o out.txt -m mapt.txt -v 1 -i demo_50.txt 
// ./data_click_table_online -s hour app_id site_id -o t_out.txt -m mapt.txt -v 1 -i demo_5h.txt
// ./data_click_table_online -o t_out.txt -m mapt.txt -v 1 -i demo_800h.txt -c 1 -e demo_100h.txt -t test_100h.txt





