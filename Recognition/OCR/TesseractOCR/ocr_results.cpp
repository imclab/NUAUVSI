/**
 * @file ocr_results.cpp
 * @brief Sets up OCR module results containers, to be used (returned by) any OCR module.
 * @author Jason Bunk
 */

#include "ocr_results.hpp"
#include "SharedUtils/SharedUtils.hpp"
#include <map>
#include <math.h>


std::string OCR_Result::GetCharacterAsString()
{
    if(character == 0)
        return std::string();

    char retstr[2];
    retstr[0]=character;
    retstr[1]=0;
    return std::string(retstr);
}

void OCR_Result::PrintMe(std::ostream* PRINTHERE)
{
    if(PRINTHERE != nullptr)
    {
        (*PRINTHERE) << "character:  " << character //<< "  (ascii #" << ((int)(character)) << ")"
            << ", confidence: " << confidence << ", relative angle to source image: "
            << relative_angle_of_character_to_source_image << std::endl;
    }
}


void OCR_ResultsContainer::EliminateDuplicates()
{
//merges duplicate letter results (from one saliency crop) from different CSEGs


    std::map<char, OCR_duplicate_eliminator>                    ocr_characters_map;
    std::map<char, OCR_duplicate_eliminator>::iterator          ocr_characters_iter;



    std::vector<OCR_Result>::iterator    ocr_iter    = results.begin();



    for(; ocr_iter != results.end(); ocr_iter++)
    {
        ocr_characters_iter = ocr_characters_map.find(ocr_iter->character);

        if(ocr_characters_iter == ocr_characters_map.end()) //not a duplicate
            ocr_characters_map[ocr_iter->character] = OCR_duplicate_eliminator();

        ocr_characters_map[ocr_iter->character].confidence_total += ocr_iter->confidence;
        ocr_characters_map[ocr_iter->character].num_instances++;
        ocr_characters_map[ocr_iter->character].angles_to_reference.push_back(ocr_iter->relative_angle_of_character_to_source_image);
    }

    results.clear();

    ocr_characters_iter = ocr_characters_map.begin();
    for(; ocr_characters_iter != ocr_characters_map.end(); ocr_characters_iter++)
    {
        results.push_back(OCR_Result(

        (ocr_characters_iter->second.confidence_total / static_cast<double>(ocr_characters_iter->second.num_instances)),
        GetMeanAngle(ocr_characters_iter->second.angles_to_reference),
        ocr_characters_iter->first

        ));
    }

    SortByConfidence();
}


void OCR_ResultsContainer::EliminateCharactersBelowConfidenceLevel(double minimum_confidence)
{
	SortByConfidence();
	std::vector<OCR_Result>::iterator iter = results.begin();
	for(; iter != results.end(); iter++)
	{
		if(iter->confidence < minimum_confidence)
			break;
	}

	if(iter != results.end())
		results.erase(iter, results.end());
}


void OCR_ResultsContainer::KeepOnlyTopFractionOfCharacters(double fraction)
{
	SortByConfidence();
	int num_to_keep = RoundDoubleToInteger(fraction * static_cast<double>(results.size()));
	if(num_to_keep > 0 && num_to_keep < results.size())
		results.erase(results.begin()+num_to_keep, results.end());
}


void OCR_ResultsContainer::GetTopCharactersWithHighestConfidences(OCR_ResultFinal*& returned_result1, OCR_ResultFinal*& returned_result2,
							double threshold_percent_difference_of_guesses_to_report_guess2,
							std::ostream* PRINTHERE/*=nullptr*/)
{
	if(returned_result1 != nullptr)
	{
		delete returned_result1; returned_result1=nullptr;
	}
	if(returned_result2 != nullptr)
	{
		delete returned_result2; returned_result2=nullptr;
	}

	std::map<char,double> character_totalconfidences;
	std::map<char,int> character_appearances;

	std::vector<OCR_Result>::iterator resiter = results.begin();
	for(; resiter != results.end(); resiter++)
	{
		character_totalconfidences[resiter->character] += resiter->confidence;
		character_appearances[resiter->character] += 1;
	}

	char chosen_char=0;		double chosen_char_total_confidence=0.0;
	char chosen_char_runnerup=0;	double chosen_char_total_confidence_runnerup=0.0;

	std::map<char,double>::iterator citer = character_totalconfidences.begin();
	for(; citer != character_totalconfidences.end(); citer++)
	{
		if(citer->second > chosen_char_total_confidence)
		{
			chosen_char_runnerup = chosen_char;
			chosen_char = citer->first;

			chosen_char_total_confidence_runnerup = chosen_char_total_confidence;
			chosen_char_total_confidence = citer->second;
		}
	}

	if(chosen_char != 0)
	{
		returned_result1 = new OCR_ResultFinal(chosen_char_total_confidence / static_cast<double>(character_appearances[chosen_char]),
							chosen_char_total_confidence,
							GetMeanAngle_FromOCRResults(results, chosen_char),
							chosen_char);
	}
	if(chosen_char_runnerup != 0)
	{
		if((fabs(chosen_char_total_confidence - chosen_char_total_confidence_runnerup) / chosen_char_total_confidence)
				< threshold_percent_difference_of_guesses_to_report_guess2)
		{
		returned_result2 = new OCR_ResultFinal(chosen_char_total_confidence_runnerup / static_cast<double>(character_appearances[chosen_char_runnerup]),
							chosen_char_total_confidence_runnerup,
							GetMeanAngle_FromOCRResults(results, chosen_char_runnerup),
							chosen_char_runnerup);
		}

        if(PRINTHERE != nullptr)
        {
            (*PRINTHERE) << "percent difference between top two guesses: "
            << (fabs(chosen_char_total_confidence - chosen_char_total_confidence_runnerup) / chosen_char_total_confidence) << std::endl;
        }
	}
}


std::vector<OCR_Result> OCR_ResultsContainer::GetTopNResults(int max_num_results_sought, double cutoff_confidence_of_final_result)
{
    std::vector<OCR_Result> returned_results(results);
    std::map<char, bool> checkmap;
    std::map<char, bool>::iterator checkmap_iter;

	SortByConfidence();


    //first: keep only the results with the minimum confidence
    for(std::vector<OCR_Result>::iterator riter = returned_results.begin(); riter != returned_results.end();)
    {
        if(riter->confidence < cutoff_confidence_of_final_result)
            riter = returned_results.erase(riter);
        else
            riter++;
    }


    //second: eliminate duplicates
    for(std::vector<OCR_Result>::iterator riter = returned_results.begin(); riter != returned_results.end();)
    {
        if(checkmap.empty())
        {
            checkmap[riter->character] = true;
            riter++;
        }
        else
        {
            checkmap_iter = checkmap.find(riter->character);

            if(checkmap_iter != checkmap.end()) //duplicate entry
                riter = returned_results.erase(riter);
            else
            {
                checkmap[riter->character] = true;
                riter++;
            }
        }
    }


    if(returned_results.size() > max_num_results_sought)
        returned_results.erase(returned_results.begin()+max_num_results_sought, returned_results.end());


    return returned_results;
}


/*static*/ double OCR_ResultsContainer::GetMeanConfidence(std::vector<OCR_Result> & theresults, char character)
{
	double conftotal = 0.0;
	double numfound = 0.0;

	std::vector<OCR_Result>::iterator iter = theresults.begin();
	for(; iter != theresults.end(); iter++)
	{
		if(iter->character == character)
		{
			conftotal += iter->confidence;
			numfound += 1.0;
		}
	}
	return conftotal / numfound;
}


/*static*/ double OCR_ResultsContainer::GetMeanAngle_FromOCRResults(std::vector<OCR_Result> & theresults, char character)
{
	double xx = 0.0;
	double yy = 0.0;

	std::vector<OCR_Result>::iterator iter = theresults.begin();
	for(; iter != theresults.end(); iter++)
	{
		if(iter->character == character)
		{
			xx += cos(iter->relative_angle_of_character_to_source_image * 0.017453292519943296); //to radians
			yy += sin(iter->relative_angle_of_character_to_source_image * 0.017453292519943296);
		}
	}

	if((fabs(xx)+fabs(yy)) < 0.000001)
		return 0.0;

	return (atan2(yy,xx) * 57.295779513082321); //back to degrees
}


double AverageTwoAngles(double angle1, double angle2)
{
	double xx = 0.0;
	double yy = 0.0;

	xx += cos(angle1 * 0.017453292519943296); //to radians
	yy += sin(angle1 * 0.017453292519943296);

	xx += cos(angle2 * 0.017453292519943296); //to radians
	yy += sin(angle2 * 0.017453292519943296);

	if((fabs(xx)+fabs(yy)) < 0.000001)
		return 0.0;

	return (atan2(yy,xx) * 57.295779513082321); //back to degrees
}

