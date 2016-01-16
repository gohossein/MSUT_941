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

void parse_program_options(const int argc, const char *const argv[], std::string& inputfile, std::string& outfile, int& beta, int& alpha_beta_type, int& alpha)
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
        ("beta,b", po::value<int>(&beta)->default_value(5),"set  beta, defualt = 5")
        ("alpha,a", po::value<int>(&alpha)->default_value(10),"set alphaa, defualt = 10")
        ("alpha_beta_type,p", po::value<int>(&alpha_beta_type)->default_value(0),"set in range (0,8) for number of grades, defualt = 0")
        ("inputfile,i", po::value<std::string>(&inputfile)->required(), "input file name")
    	("outfile,o", po::value<std::string>(&outfile)->required(), "output file name")
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
    
/*----------------------------------------------------------------------------------------------

	Start Section

----------------------------------------------------------------------------------------------*/


    time_t start, end, start_train_sub, end_train_sub, start_test_sub, end_test_sub, start_alpha_beta_type, end_alpha_beta_type;
    time(&start);
    std::string inputfilename, outputfilename, line, header_names_orig, tempname, tempname2;
    std::vector<std::string> list_skiped_cols, data_row, header_orig, header_col_names, header_col_names_two;
    int beta, alpha, alpha_beta_type, col_index, data_target;
    std::ifstream csv_file;
    std::ofstream write_sub, write_sub_test;
    parse_program_options(argc, argv, inputfilename, outputfilename, beta, alpha_beta_type, alpha);
    float grade_click;

/*-------------------------------------------------------------------------------------------

	read file line by line , grade file


-------------------------------------------------------------------------------------------*/


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
	csv_file.clear(); //Clear State
    csv_file.seekg(0,std::ios::beg);
  	
	boost::split( header_orig, line, boost::is_any_of(","), boost::token_compress_on );
	
	write_sub.open(outputfilename);
	write_sub.precision(0);
	write_sub.setf(std::ios_base::fixed);
	while(getline(csv_file,line))
	{
		
		boost::split( data_row, line, boost::is_any_of(","), boost::token_compress_on );
		col_index = 0;
		write_sub << data_row[0];
		data_row.erase(data_row.begin());
    	for (auto & element : data_row)
    	{
    		if (alpha_beta_type == 0)
            {
                if((atof(element.c_str()) * 100) < (alpha))
                {
                    grade_click = 0;
                }else{
                    grade_click = ceil((atof(element.c_str()) * float(alpha))) ;
                }

            }else if(alpha_beta_type == 1)
            {
                if((atof(element.c_str()) * 100) < (alpha))
                {
                    grade_click = 0;
                }else{
                    grade_click = ceil(log((atof(element.c_str()) * float(alpha)))) ;
                }

            // }else if(alpha_beta_type == 2)
            // {
            //     if((atof(element.c_str()) * 100) < (50))
            //     {
            //         grade_click = 0;
            //     }else if((atof(element.c_str()) * 100) < (100 )){
            //         grade_click = 1 ;
            //     }else{
            //         grade_click = alpha;
            //     }
                

            // }else if(alpha_beta_type == 3)
            // {
            //     if((atof(element.c_str()) * 100) < (50 - beta))
            //     {
            //         grade_click = 0;
            //     }else if((atof(element.c_str()) * 100) < (100 - beta)){
            //         grade_click = 0.2 * alpha  ;
            //     }else{
            //         grade_click = alpha;
            //     }
               

            // }else if(alpha_beta_type == 4)
            // {
            //     if((atof(element.c_str()) * 100) < (50 - beta))
            //     {
            //         grade_click = 0;
            //     }else if((atof(element.c_str()) * 100) < (100 - beta)){
            //         grade_click = alpha / float(beta) ;
            //     }else{
            //         grade_click = alpha;
            //     }
              

            // }else if(alpha_beta_type == 5)
            // {
            //     if((atof(element.c_str()) * 100) < (50 - beta))
            //     {
            //         grade_click = 0;
            //     }else if((atof(element.c_str()) * 100) < (100 - beta)){
            //         grade_click = log(alpha / float(beta)) ;
            //     }else{
            //         grade_click = alpha * log(alpha / float(beta));
            //     }
               

            // }else if(alpha_beta_type == 6)
            // {
            //     if((atof(element.c_str()) * 100) < (50 - beta))
            //     {
            //         grade_click = 0;
            //     }else if((atof(element.c_str()) * 100) < (100 - beta)){
            //         grade_click = alpha / float(beta) ;
            //     }else{
            //         grade_click = alpha * alpha / float(beta);
            //     }
                

            // }else if(alpha_beta_type == 7)
            // {
            //     if((atof(element.c_str()) * 100) < (50 - beta))
            //     {
            //         grade_click = 0;
            //     }else if((atof(element.c_str()) * 100) < (100 - beta)){
            //         grade_click = alpha  ;
            //     }else{
            //         grade_click = pow(alpha,beta);
            //     }
               

            // }else if(alpha_beta_type == 8){
            //     if((atof(element.c_str()) * 100) < (50 - beta))
            //     {
            //         grade_click = log(alpha / float(beta)) / float(100);
            //     }else if((atof(element.c_str()) * 100) < (100 - beta)){
            //         grade_click = log(alpha / float(beta)) ;
            //     }else{
            //         grade_click = alpha * log(alpha / float(beta));
            //     }
            // }else if(alpha_beta_type == 9){
            //     if((atof(element.c_str()) * 100) < (50 - beta))
            //     {
            //         grade_click = 0;
            //     }else{
            //         grade_click = 1 ;
            //     }
            // }else 
            // {
            //     if((atof(element.c_str()) * 100) < (50 - beta))
            //     {
            //         grade_click = 0;
            //     }else {
            //         grade_click = alpha / float(beta) ;
            //     }

            }

			write_sub << "," << grade_click;
		}
		write_sub << std::endl;
    	data_row.clear();
				
    }
	write_sub.close();
	tempname.clear();
	csv_file.close();



/*----------------------------------------------------------------------------------------------

	End Section

----------------------------------------------------------------------------------------------*/





	time(&end);
	std::cout<<"Total Time: "<<difftime(end,start)<<std::endl;
	return 0;

}



//compile: g++ -std=c++11 -lboost_program_options -o ND New_Distribution_Grade.cpp
//./ND -o grade.csv -i temp.csv -p 0 -a 20 -b 5





