#ifndef ___OCR_MODULE_MAIN_H____
#define ___OCR_MODULE_MAIN_H____

#include <opencv/cv.h>
#include "ocr_results.hpp"
class test_data_results_ocr;
class OCRModuleAlgorithm_Template;


class TessOCR_Module_Main
{
public:
    OCR_ResultsContainer last_obtained_results;

    OCRModuleAlgorithm_Template* my_ocr_algorithm;


    TessOCR_Module_Main();


	void DoModule(std::vector<cv::Mat>* input_CSEGs,

        std::ostream* PRINT_TO_FILE_HERE=nullptr,
        std::string* folder_path_of_output_saved_images=nullptr,
		bool save_images_and_results=false,
		std::string* name_of_target_image=nullptr,
		test_data_results_ocr* optional_results_info=nullptr,
		std::string* correct_shape_name=nullptr,
		const char* correct_ocr_character=nullptr);


protected:
	void Attempt_OCR_OnOneCSEG(cv::Mat input_CSEG,

		bool save_images_and_results=false,
		std::string* name_of_target_image=nullptr,
		test_data_results_ocr* optional_results_info=nullptr,
		std::string* correct_shape_name=nullptr,
		const char* correct_ocr_character=nullptr,
		int test_number=-1);
};

#endif
