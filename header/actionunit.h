#ifndef ACTION_UNIT_H
#define ACTION_UNIT_H
#include <array>
#include <vector>
#include <string>
#include <opencv2/core/core.hpp>
#include <stdint.h>
#include <fstream>
#include <iostream>

static constexpr std::array<const char*,28> ActionUnitName = {
        "Inner Brow Raiser",
        "Outer Brow Raiser",
        "",
        "Brow Lowerer",
        "Upper Lid Raiser",
        "Cheek Raiser",
        "Lid Tightener",
        "",
        "Nose Wrinkler",
        "Upper Lid Raiser",
        "Nasolabial Fold Deepener",
        "Lip Corner Puller",
        "Cheeck Puffer",
        "Dimpler",
        "Lip Corner Depressor",
        "Lower Lip Depressor",
        "Chin Raiser",
        "Lip Puckerer",
        "",
        "Lip Stretcher",
        "",
        "Lip Funneler",
        "Lip Tightner",
        "Lip Pressor",
        "Lips Part",
        "Jaw Drop",
        "Mouth Stretch",
        "Lip Suck"
};

struct ActionUnit{
public:

        /**
         * Returns the name/label of the actions.
         */
        inline std::vector<std::string> getActionsAsName() const{
                auto numbers = getActionsAsNumber();
                std::vector<std::string> result;
                result.reserve(numbers.size());
                for(uint8_t i: numbers){
                        if (i<0 || i>ActionUnitName.size())
                                result.push_back("?");
                        else
                                result.push_back(ActionUnitName[i-1]);
                }
                return result;
        }

        static int getActionNumberFromName(std::string name) {
                for (int i = 0; i < 28; ++i) {
                        if (ActionUnitName[i] == name) return i + 1;
                }
                return -1;
        }

        /**
         * Returns the numbers of the actions.
         */
        inline std::vector<int>    getActionsAsNumber() const{
                return getActionsIntensity(0);
        }
        //Frame is starting at 1
        //Frame=0 returns the header
        /**
         * Get the intensity of every action for a specific frame.
         * The result V is a vector of int.
         * V[i] is the intensity of the action i.
         * For the name of the action see getActionsAsName()[i].
         */
        inline std::vector<int>    getActionsIntensity(int frame) const{
                std::vector<int> result;
                int cols = this->mat.cols;
                if (this->mat.rows < frame) return result;
                result.reserve(cols);
                const uint8_t* firstrow = this->mat.ptr(frame);
                for(int i=0; i<cols; i++){
                        result.push_back(firstrow[i]);
                }
                return result;
        }

        inline int size() const{
            return mat.rows-1;
        }

        bool getActionByName(const std::string& name, cv::Mat &output)
        {
            int action_idx = -1;
            for (int i = 0; i < getActionsAsName().size(); i++)
            {
                if (getActionsAsName()[i] == name){
                    action_idx = i;
                    break;
                }
            }
            if(action_idx == -1)
                return false;
            output = this->mat(cv::Rect(action_idx, 1, 1, mat.rows - 1)).clone();
            return true;
        }

        // First Row -> describes the action
        // Other rows (row is frame) -> intensity of action
        cv::Mat_<uint8_t> mat;
};

/**
 * Parse a text file from an action unit.
 * @param stream input stream of the text file
 * @return ActionUnit. On Fail an uninitialized ActionUnit.
 */
inline ActionUnit readActionUnitFromFile(std::istream & stream){
        if (!stream.good()) return ActionUnit{};

        // Reading file line by line
        // split every line using ':' as separator.
        std::string line;
        std::vector<std::vector<std::string>> strmatrix;
        while (std::getline(stream,line)){
                std::vector<std::string> strrow;
                int beg =0; int end=0;
                for(; end < line.length(); end++){
                        if (line[end]==':' ||line[end]=='\n'){
                                strrow.push_back(line.substr(beg,end));
                                beg = end+1;
                        }
                }
                if (end == line.length())
                        strrow.push_back(line.substr(beg,line.length()));
                strmatrix.push_back(strrow);
        }
        //Convert the strings into integers.
        //Ignoring first row
        int rows = strmatrix.size();
        if (rows == 0) return ActionUnit{};
        int cols = strmatrix[0].size()-1;
        if (cols < 0) return ActionUnit{};
        cv::Mat_<uint8_t> mat(rows,cols);
        for(int y=0; y<rows; y++){
                uint8_t* r = mat.ptr(y);
                std::vector<std::string>& strrow = strmatrix[y];
                if (strrow.size() != cols+1){
                        return ActionUnit{};
                }
                for (int x=0; x<cols; x++){
                        try{
                                int i = std::stoi(strrow[x+1]);
                                r[x] =  i;
                        }catch (std::invalid_argument&){ // Not a number
                                r[x] = 0;
                        }
                }
        }
        return ActionUnit{mat};
}

/**
 * Parse a text file from an action unit.
 * @param filename the filename of the action unit
 * @return ActionUnit. On Fail an uninitialized ActionUnit.
 */
inline ActionUnit readActionUnitFromFile(const std::string & filename){
        std::ifstream stream(filename);
        auto res = readActionUnitFromFile(stream);
        return res;
}

inline ActionUnit readActionUnitFromFolder(const std::string& folderpath)
{
    std::vector<std::string> filelist;
    cv::glob(folderpath + "/*AUs.txt", filelist, true);
    cv::Mat_<uint8_t> actionMatrix;

    for(auto &filename : filelist)
    {
        ActionUnit unit = readActionUnitFromFile(filename);
        if(actionMatrix.rows == 0)
        {
            actionMatrix = unit.mat.clone();
        }
        else
        {
            // concatenate matrices
            cv::vconcat(actionMatrix, unit.mat(cv::Rect(0, 1, unit.mat.cols, unit.mat.rows - 1)), actionMatrix);
        }
    }
    ActionUnit unit{actionMatrix};
    return unit;
}

#endif
