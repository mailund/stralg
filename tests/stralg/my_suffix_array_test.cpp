#include <fstream>
#include <remap.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <sstream>
#include <vector>
#include <iostream>

extern "C" {
	#include <suffix_array.h>
}

/*
 *static void test_order(struct suffix_array *sa) {
 *    for (uint32_t i = 1; i < sa->length; ++i)
 *        assert(strcmp((char *)(sa->string + sa->array[i-1]),
 *                      (char *)(sa->string + sa->array[i]))
 *               < 0);
 *}
 *
 *static void test_inverse(struct suffix_array *sa) {
 *    compute_inverse(sa);
 *    for (uint32_t i = 0; i < sa->length; ++i) {
 *        assert(sa->inverse[sa->array[i]] == i);
 *        assert(sa->array[sa->inverse[i]] == i);
 *    }
 *}
 *
 *static int lcp(const uint8_t *a, const uint8_t *b) {
 *    int l = 0;
 *    while (*a && *b && *a == *b) {
 *        ++a; ++b; ++l;
 *    }
 *    return l;
 *}
 *
 *static void test_lcp(struct suffix_array *sa) {
 *    compute_lcp(sa);
 *
 *    //assert(sa->lcp[0] == sa->lcp[sa->length]);
 *    assert(sa->lcp[0] == 0);
 *
 *    for (uint32_t i = 1; i < sa->length; ++i) {
 *        uint32_t l = lcp(sa->string + sa->array[i-1], sa->string + sa->array[i]);
 *        assert(sa->lcp[i] == l);
 *    }
 *}
 *
 */
//print sa, isa (inverse), lcp
static void print_arrays(struct suffix_array *sa, uint8_t *cad){
    for (uint32_t i = 0; i < sa->length; ++i)
        printf("sa[%3d] == %3u\t%s\n", i, sa->array[i], cad + sa->array[i]);
    printf("\n");

    for (uint32_t i = 0; i < sa->length; ++i)
        printf("isa[%3d] == %3u\t%s\n", i, sa->inverse[i], cad + i);
    printf("\n");

    for (uint32_t i = 0; i < sa->length; ++i)
        printf("lcp[%3d] == %3u\t%s\n", i, sa->lcp[i], cad + sa->array[i]);
    printf("\n");
 }

//save sa, isa, lcp in files
static void save_arrays(struct suffix_array *sa, uint8_t *cad, std::string filename){
	std::cout << "[" << cad << "]" << std::endl;

	std::ofstream output_sa{filename + ".sa"};
    for (uint32_t i = 0; i < sa->length; ++i)
		output_sa << sa->array[i] << std::endl;
		//output_sa << i << "," << sa->array[i] << "," << (cad + sa->array[i]) << std::endl;
	output_sa.close();

	std::ofstream output_isa{filename + ".isa"};
    for (uint32_t i = 0; i < sa->length; ++i)
		output_isa << sa->inverse[i] << std::endl;
		//output_isa << i << "," << sa->inverse[i] << "," << (cad + i) << std::endl;
	output_isa.close();

	std::ofstream output_lcp{filename + ".lcp"};
    for (uint32_t i = 0; i < sa->length; ++i)
		output_lcp << sa->lcp[i] << std::endl;
		//output_lcp << i << "," << sa->lcp[i] << "," << (cad + sa->array[i]) << std::endl;
	output_lcp.close();
}

static std::pair<int, int> search_pattern(struct suffix_array *sa, uint8_t *pattern){
    int l_idx = lower_bound_search(sa, pattern);
    int u_idx = upper_bound_search(sa, pattern);
	return {l_idx, u_idx};
}

static void my_tests(uint8_t* text, std::vector<std::string> patterns, std::string filename){

	printf("text: [%s]\n\n\n", text);

    struct suffix_array *sa;
    sa = skew_sa_construction(text);
    compute_lcp(sa);
	print_arrays(sa, text);
   
	//test_order(sa);
	//test_inverse(sa);
	//test_lcp(sa);

	save_arrays(sa, text, filename);

	std::ofstream patterns_idxs{filename + ".idxs"};
	for(auto &pattern: patterns){
		auto idx = search_pattern(sa, (uint8_t *)pattern.c_str());
		std::cout << idx.first << "," << idx.second << " : pattern: " << pattern << std::endl;
		patterns_idxs << idx.first << "," << idx.second << " [" << pattern << "]" << std::endl;
	}
	patterns_idxs.close();

    free_suffix_array(sa);
	printf("======================================== \n");
}

int main(int argc, char *argv[]) {
	if(argc < 3){
		std::cerr << "Error: it is required two parameters for this test" << std::endl;
		std::cerr << "use: " << argv[0] << " <input_text_file> <intput_patters_file>" << std::endl;
		exit(-1);
	}

	std::string text_file_name{argv[1]};
	std::string patterns_file_name{argv[2]};

	std::cout << "Input text file name: " << text_file_name << std::endl;
	std::cout << "Input patterns file name: " << patterns_file_name << std::endl;

	std::ifstream input_text{argv[1]};
	std::ifstream input_patterns{argv[2]};

	if(!input_text){
		std::cerr << text_file_name + ": Error opening file" << std::endl;
		perror(text_file_name.c_str());
		exit(-1);
	}

	if(!input_patterns){
		std::cerr << patterns_file_name + ": Error opening file" << std::endl;
		perror(patterns_file_name.c_str());
		exit(-1);
	}

	std::string line;
	std::vector<std::string> patterns;
	while (getline(input_patterns, line)) 
		patterns.push_back(line);

	std::stringstream buffer;
	buffer << input_text.rdbuf();
	std::cout << buffer.str().c_str() << std::endl;

	my_tests((uint8_t*)buffer.str().c_str(), patterns, text_file_name);
    return EXIT_SUCCESS;
}
