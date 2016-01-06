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

void parse_program_options(const int argc, const char *const argv[], std::string& infile, std::string& intestfile, std::string& outfile, std::string& testfile, std::string& extension, std::vector<std::string>& ignorecols, int& skipheader, int& verbose, int& nosub, int& percent, int& lengthofsub, int& lengthoftestsub, int& typeofsub, double& downsize)
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
        ("kind,k", po::value<int>(&typeofsub)->default_value(1),"0 subsample type, with skip cols = 0, without skip cols = 1, split test train with percent without skipcols= 2, with skipcols = 3, test subsampling = 4")
        ("skipheader,s", po::value<int>(&skipheader)->default_value(0),"set 1 for skip header, defualt = 0")
        ("percent,p", po::value<int>(&percent)->default_value(20),"set in range (1,99) for test subsample percent, defualt = 20")
        ("nosub,n", po::value<int>(&nosub)->default_value(8),"number of Subsamples defualt = 8")
        ("lengthofsub,l", po::value<int>(&lengthofsub)->default_value(10000),"length of Subsamples defualt = 10000")
        ("lengthoftestsub,u", po::value<int>(&lengthoftestsub)->default_value(10000),"length of test Subsamples defualt = 10000")
    	("infile,i", po::value<std::string>(&infile)->required(), "input file name")
    	("intestfile,f", po::value<std::string>(&intestfile), "input test file name")
    	("outfile,o", po::value<std::string>(&outfile)->required(), "subsample file name, example: test1.csv, test2.csv --> test")
    	("testfile,t", po::value<std::string>(&testfile), "output test file name, example: test1.csv, test2.csv --> test")
    	("extension,e", po::value<std::string>(&extension)->required(), "file extension such as txt csv tsv vw libsvm ...")
    	("colsskip,c", po::value<std::vector<std::string>>(&ignorecols)->multitoken(), "list of columns to be skiped, use space  for item seperator")
    	("downsize,d", po::value<double>(&downsize)->default_value(0.5),"down sampling rate, defualt = 0.5")
    ;
// ./Subsampling -c app_id site_id -o train -e csv -v 1 -n 4 -l 1000 -k 1 -i demo_50.txt
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
    
/*----------------------------------------------------------------------------------------------

	Start Section

----------------------------------------------------------------------------------------------*/


    time_t start, end, start_train_sub, end_train_sub, start_test_sub, end_test_sub, start_percent, end_percent;
    time(&start);
    double downsize;
    std::string inputfilename, inputtestfilename, outputfilename, extension, testfilename, outputtestfilename, line, header_names_orig, tempname, tempname2;
    std::vector<std::string> list_skiped_cols, data_row, header_orig, header_col_names, header_col_names_two;
    int verbose, skipheader, nosub, lengthofsub, lengthoftestsub, typeofsub, percent, col_index, data_target;
    std::ifstream csv_file;
    std::ofstream write_sub, write_sub_test;
    parse_program_options(argc, argv, inputfilename, inputtestfilename, outputfilename, testfilename, extension, list_skiped_cols, skipheader, verbose, nosub, percent, lengthofsub, lengthoftestsub, typeofsub, downsize);




/*-------------------------------------------------------------------------------------------

	read file line by line , write to subsample train file


-------------------------------------------------------------------------------------------*/

	time(&start_train_sub);
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
	header_names_orig = line;

  	if(typeofsub == 1)
  	{
	    for(int i=0; i<nosub; ++i)
	    {
	    	tempname = outputfilename + "." + std::to_string(i+1) + "." + extension;
	    	write_sub.open(tempname);
	    	if (!skipheader)
	    		write_sub << header_names_orig << std::endl;

	    	for(int j=0; j<lengthofsub; ++j)
	    	{
	    		getline(csv_file,line);
	    		if (line.at(0) == '1')
	    			write_sub << line << std::endl;
	    		else if ( downsize > ((double) rand() / (RAND_MAX)))
	    			write_sub << line << std::endl;
	    	}
	    	write_sub.close();
	    	tempname.clear();
	    }
	}else if(typeofsub == 0){
		boost::split( header_orig, line, boost::is_any_of(","), boost::token_compress_on );
		for(int i=0; i<nosub; ++i)
	    {
	    	tempname = outputfilename + "." + std::to_string(i+1) + "." + extension;
	    	write_sub.open(tempname);
	    	if(!skipheader)
	    	{
	    		header_col_names = header_orig;
		    	col_index = 1;
		    	for (auto & element : header_col_names)
		    	{
		    		if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), element) == list_skiped_cols.end())
		    		{
		    			write_sub << element;
		    			if ( col_index != header_col_names.size())
		    				write_sub << ",";	
		    		}
		    		col_index ++;
		    	}
		    	write_sub << std::endl;	
	    	}
	    	
	    	for(int j=0; j<lengthofsub; ++j)
	    	{
	    		getline(csv_file,line);
	    		boost::split( data_row, line, boost::is_any_of(","), boost::token_compress_on );
	    		col_index = 0;
		    	for (auto & element : data_row)
		    	{
		    		if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), header_orig[col_index]) == list_skiped_cols.end())
		    		{
		    			write_sub << element;
		    			if ( (col_index+1) != header_orig.size())
		    				write_sub << ",";		
		    		}
		    		col_index ++;
		    	}
		    	write_sub << std::endl;
		    	data_row.clear();
	    	}
	    	write_sub.close();
	    	tempname.clear();
	    	header_col_names.clear();
	    }
	}
	csv_file.close();
  	time(&end_train_sub);

/*----------------------------------------------------------------------------------------------

	read file line by line , write to subsample test file

----------------------------------------------------------------------------------------------*/

	time(&start_test_sub);
	if( typeofsub == 4)
	{
		
		csv_file.open(inputtestfilename.c_str());
	    if(!csv_file.is_open())
	    {
	        std::cout<<"input file not found!"<<std::endl;
	        return 0;
	    }
		csv_file.clear(); //Clear State
	    csv_file.seekg(0,std::ios::beg);
	   
		  	
	    // read headers
	    getline(csv_file,line);
		header_names_orig = line;

	  	if(typeofsub ==1)
	  	{
		    for(int i=0; i<nosub; ++i)
		    {
		    	tempname = testfilename + "." + std::to_string(i+1) + "." + extension;
		    	write_sub.open(tempname);
		    	if (!skipheader)
		    		write_sub << header_names_orig << std::endl;

		    	for(int j=0; j<lengthoftestsub; ++j)
		    	{
		    		getline(csv_file,line);
		    		write_sub << line << std::endl;
		    	}
		    	write_sub.close();
		    	tempname.clear();
		    }
		}else if(typeofsub == 0){
			boost::split( header_orig, line, boost::is_any_of(","), boost::token_compress_on );
			for(int i=0; i<nosub; ++i)
		    {
		    	tempname = testfilename + "." + std::to_string(i+1) + "." + extension;
		    	write_sub.open(tempname);
		    	if(!skipheader)
		    	{
		    		header_col_names = header_orig;
			    	col_index = 1;
			    	for (auto & element : header_col_names)
			    	{
			    		if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), element) == list_skiped_cols.end())
			    		{
			    			write_sub << element;
			    			if ( col_index != header_col_names.size())
			    				write_sub << ",";	
			    		}
			    		col_index ++;
			    	}
			    	write_sub << std::endl;	
		    	}
		    	
		    	for(int j=0; j<lengthoftestsub; ++j)
		    	{
		    		getline(csv_file,line);
		    		boost::split( data_row, line, boost::is_any_of(","), boost::token_compress_on );
		    		col_index = 0;
			    	for (auto & element : data_row)
			    	{
			    		if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), header_orig[col_index]) == list_skiped_cols.end())
			    		{
			    			write_sub << element;
			    			if ( (col_index+1) != header_orig.size())
			    				write_sub << ",";		
			    		}
			    		col_index ++;
			    	}
			    	write_sub << std::endl;
			    	data_row.clear();
		    	}
		    	write_sub.close();
		    	tempname.clear();
		    	header_col_names.clear();
		    }
		}
		csv_file.close();
	  	
	}
	
	time(&end_test_sub);


/*----------------------------------------------------------------------------------------------

	split data to train - test with percent

----------------------------------------------------------------------------------------------*/

	time(&start_percent);
	lengthoftestsub = lengthofsub * percent / 100;
	lengthofsub = lengthofsub - lengthoftestsub;
	csv_file.open(inputfilename.c_str());
    if(!csv_file.is_open())
    {
        std::cout<<"input file not found!"<<std::endl;
        return 0;
    }
	csv_file.clear(); //Clear State
    csv_file.seekg(0,std::ios::beg);
    getline(csv_file,line);

	if(typeofsub == 2)
  	{

	    for(int i=0; i<nosub; ++i)
	    {
	    	tempname = outputfilename + "." + std::to_string(i+1) + "." + extension;
	    	tempname2 = testfilename + "." + std::to_string(i+1) + "." + extension;
	    	write_sub.open(tempname);
	    	write_sub_test.open(tempname2);
	    	if (!skipheader)
	    	{
	    		write_sub << header_names_orig << std::endl;
	    		write_sub_test << header_names_orig << std::endl;
	    	}

	    	for(int j=0; j<lengthofsub; ++j)
	    	{
	    		getline(csv_file,line);
	    		write_sub << line << std::endl;
	    	}
	    	write_sub.close();
	    	tempname.clear();
	    	for(int j=0; j<lengthoftestsub; ++j)
	    	{
	    		getline(csv_file,line);
	    		write_sub_test << line << std::endl;
	    	}
	    	write_sub_test.close();
	    	tempname2.clear();
	    }
	}else if(typeofsub == 3){
		boost::split( header_orig, line, boost::is_any_of(","), boost::token_compress_on );
		for(int i=0; i<nosub; ++i)
	    {
	    	tempname = outputfilename + "." + std::to_string(i+1) + "." + extension;
	    	tempname2 = testfilename + "." + std::to_string(i+1) + "." + extension;
	    	write_sub_test.open(tempname2);
	    	write_sub.open(tempname);
	    	if(!skipheader)
	    	{
	    		header_col_names = header_orig;
		    	col_index = 1;
		    	for (auto & element : header_col_names)
		    	{
		    		if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), element) == list_skiped_cols.end())
		    		{
		    			write_sub << element;
		    			write_sub_test << element;
		    			if ( col_index != header_col_names.size())
		    			{
		    				write_sub << ",";	
		    				write_sub_test << ",";
		    			}
		    		}
		    		col_index ++;
		    	}
		    	write_sub << std::endl;
		    	write_sub_test << std::endl;	
	    	}
	    	
	    	for(int j=0; j<lengthofsub; ++j)
	    	{
	    		getline(csv_file,line);
	    		boost::split( data_row, line, boost::is_any_of(","), boost::token_compress_on );
	    		col_index = 0;
		    	for (auto & element : data_row)
		    	{
		    		if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), header_orig[col_index]) == list_skiped_cols.end())
		    		{
		    			write_sub << element;
		    			if ( (col_index+1) != header_orig.size())
		    				write_sub << ",";		
		    		}
		    		col_index ++;
		    	}
		    	write_sub << std::endl;
		    	data_row.clear();
	    	}
	    	write_sub.close();
	    	tempname.clear();
	    	for(int j=0; j<lengthoftestsub; ++j)
	    	{
	    		getline(csv_file,line);
	    		boost::split( data_row, line, boost::is_any_of(","), boost::token_compress_on );
	    		col_index = 0;
		    	for (auto & element : data_row)
		    	{
		    		if (std::find(list_skiped_cols.begin(), list_skiped_cols.end(), header_orig[col_index]) == list_skiped_cols.end())
		    		{
		    			write_sub_test << element;
		    			if ( (col_index+1) != header_orig.size())
		    				write_sub_test << ",";		
		    		}
		    		col_index ++;
		    	}
		    	write_sub_test << std::endl;
		    	data_row.clear();
	    	}
	    	tempname2.clear();
	    	write_sub_test.close();
	    	header_col_names.clear();
	    }
	}
	csv_file.close();
  	time(&end_percent);






/*----------------------------------------------------------------------------------------------

	End Section

----------------------------------------------------------------------------------------------*/





	time(&end);
	std::cout<<"Total Time: "<<difftime(end,start)<<std::endl;
	if(verbose)
	{
		std::cout<<"Train Subsampling Time: "<<difftime(end_train_sub,start_train_sub)<<std::endl;
		std::cout<<"Test Subsampling Time: "<<difftime(end_test_sub,start_test_sub)<<std::endl;
		std::cout<<"Split Test Train Subsampling Time: "<<difftime(end_percent,start_percent)<<std::endl;
	}
	return 0;

}



//compile: g++ -std=c++11 -lboost_program_options -o NegativeDownSampling NegativeDownSampling.cpp
//run: ./NegativeDownSampling -o train -e csv -v 1 -n 4 -l 30 -k 1 -i temp.txt -d 0.5
//./NegativeDownSampling -c id -o train -e csv -v 1 -n 8 -l 5000000 -k 1 -i train.csv -p 15 -t test -d 0.5
//./NegativeDownSampling -o train -e csv -v 1 -n 1 -l 5000000 -k 2 -i train.csv -p 15 -t test 





