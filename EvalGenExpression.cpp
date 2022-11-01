// EvalGenExpression.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <regex> 
#include <stack>
#include <queue>
#include <map>
#include <algorithm>
#include <vector>

using namespace std;
#include "EvalGenExpression.h"

int main(int argc, char* argv[])
{
    fstream inFile;
    bool continueProcessing = true;
    string line;
    bool partOfIfBlock = false;
    int ifBlockLineCnt = 0;
    vector <string> condBlockStr;
    
    
    if (argc != 2)
    {
        cout << "Please specify an input Python file..." << endl;
        exit(-1);
    }

    inFile.open(argv[1], ios::in);
    
    // inFile.open("C:\\Code\\EvalGenExpression\\Debug\\tc1.py");

    if (inFile.is_open())
    {
        // First scan the whole file for any errors
        continueProcessing = errorFree(inFile);

        if (continueProcessing)
        {
            // Go back to beginning of the python file
            inFile.clear();
            inFile.seekg(0, ios::beg);

            while (getline(inFile, line))
            {
                // If first character is a # then it's a comment, skip the whole line
                if (!line.empty() && line.at(0) != '#')
                {
                    if (line.find(lambdaStr) != string::npos)
                    {
                        handleLambdaStmt(line);
                        exit(0);
                    }
                    else
                    {
                        if (partOfIfBlock == false)
                        {
                            // if (line.find(eqStr) != string::npos)
                            if ((line.find(eqStr) != string::npos) && (line.find(ifStr) == string::npos))
                            {
                                // Find "=" here. Next determine whether it's simple assignment
                                // statement or arithematic operations before assignment to a result
                                // The second condition is to filter out >= or <= which has an equal, but
                                // it's not an assignment
                                handleAssignmentStmt(line);
                            }
                            else if (line.find(ifStr) != string::npos)
                            {
                                // Find a conditional block starting with an if statement, treat this
                                // and following 3 lines are part of the conditional block
                                condBlockStr.push_back(line);
                                partOfIfBlock = true;
                                ifBlockLineCnt++;
                            }
                            else if (line.find(printStr) != string::npos)
                            {
                                handlePrintStmt(line);
                            }                            
                        }
                        else
                        {
                            condBlockStr.push_back(line);
                            ifBlockLineCnt++;

                            if (ifBlockLineCnt == 4)
                            {
                                // Reset the bool so the next line will go back to main loop
                                partOfIfBlock = false;
                                ifBlockLineCnt = 0;
                                handleIfBlock(condBlockStr);
                            }
                        }
                    }
                }
            }
            inFile.close();
        }
        else
        {
            cout << "There's an error in the input Python file." << endl;
        }
    }

    return 0;
}

void handleIfBlock(vector<string> ifBlockStr)
{
    string line;
    int lineNum;
    int pos;
    int cond;
    string oper1;
    string oper2;
    int oper1Val = 0;
    int oper2Val = 0;
    bool ifBlockIsTrue = false;
    map<string, string>::iterator itr;


    line = ifBlockStr.at(0);

    if (line.find(GEStr) != string::npos)
    {
        cond = cond_GE;
    }
    else if (line.find(LEStr) != string::npos)
    {
        cond = cond_LE;
    }
    else if (line.find(GTStr) != string::npos)
    {
        cond = cond_GT;
    }
    else if (line.find(LTStr) != string::npos)
    {
        cond = cond_LT;
    }

    lineNum = 0;
    
    for (auto i = ifBlockStr.begin(); i != ifBlockStr.end(); i++)
    {
        line = string(*i);
        if (debugPrint) cout << line << endl;

        regex words_regex(regstrg);

        auto words_begin = sregex_iterator(line.begin(), line.end(), words_regex);
        auto words_end = sregex_iterator();

        // Reset pos for each line
        pos = 0;
        for (sregex_iterator k = words_begin; k != words_end; ++k)
        {
            smatch match = *k;
            string match_str = match.str();
            trim(match_str);
            
            if (lineNum == 0)
            {
                // Operand 1 is always in pos 1
                if (pos == 1)
                {
                    oper1 = match_str;
                }

                // Operand 2 is in pos 3 for LT or GT, and is in pos 4 for LE or GE
                if ((pos == 3) && ((cond == cond_LT) || (cond == cond_GT)))
                {
                    oper2 = match_str;
                }
                else if ((pos == 4) && ((cond == cond_LE) || (cond == cond_GE)))
                {
                    oper2 = match_str;
                }
                else if (match_str.compare(sliceStr) == 0)
                {
                    // Reach the end of first line, we can decide whether the if statement 
                    // is true or else statement is true
                    // First find out int value of oper1
                    itr = table.find(oper1);

                    if (itr != table.end())
                    {
                        // table maps string to string. So cast return value to int
                        oper1Val = stoi(itr->second);
                        if (debugPrint) cout << oper1 << ": " << oper1Val << endl;
                    }
                    else
                    {
                        // Can't find oper1 on the map. Check if it's just an integer
                        if (isNumber(oper1))
                        {
                            oper1Val = stoi(oper1);
                        }
                    }

                    itr = table.find(oper2);

                    if (itr != table.end())
                    {
                        oper2Val = stoi(itr->second);
                        if (debugPrint) cout << oper2 << ": " << oper2Val << endl;
                    }
                    else
                    {
                        // Can't find oper1 on the map. Check if it's just an integer
                        if (isNumber(oper2))
                        {
                            oper2Val = stoi(oper2);
                        }
                    }


                    // Test whether the condition is true
                    switch (cond)
                    {
                        case cond_GE:
                            if (oper1Val >= oper2Val) ifBlockIsTrue = true;
                            break;
                        case cond_LE:
                            if (oper1Val <= oper2Val) ifBlockIsTrue = true;
                            break;
                        case cond_GT:
                            if (oper1Val > oper2Val) ifBlockIsTrue = true;
                            break;
                        case cond_LT:
                            if (oper1Val < oper2Val) ifBlockIsTrue = true;
                            break;
                        default:
                            break;
                    }
                }

                pos++;
            }
            else
            {
                
                if (ifBlockIsTrue == true)
                {
                    if (lineNum == 1)
                    {
                        if (debugPrint) cout << "Handle line 1, " << oper1 << ": " << oper1Val << ", " << oper2 << ": " << oper2Val << endl;

                        if (ifBlockStr.at(lineNum).find(printStr) != string::npos)
                        {
                            handlePrintStmt(ifBlockStr.at(lineNum));
                        }
                        else
                        {
                            handleAssignmentStmt(ifBlockStr.at(lineNum));
                        }
                        break;
                    }
                }
                else
                {
                    if (lineNum == 3)
                    {
                        if (debugPrint) cout << "Handle line 3, " << oper1 << ": " << oper1Val << ", " << oper2 << ": " << oper2Val << endl;

                        if (ifBlockStr.at(lineNum).find(printStr) != string::npos)
                        {
                            handlePrintStmt(ifBlockStr.at(lineNum));
                        }
                        else
                        {
                            handleAssignmentStmt(ifBlockStr.at(lineNum));
                        }
                        break;
                    }
                }                
            }
            // lineNum++;
        }
        
        pos = 0;
        lineNum++;
    }

    return;
}

bool errorFree(fstream &inFile)
{
    bool noError = true;
    int openBraketNum = 0;
    int closeBraketNum = 0;
    int openSqBraketNum = 0;
    int closeSqBraketNum = 0;
    string line;

    regex words_regex(regstrg);

    while (getline(inFile, line))
    {
        // If first character is a # then it's a comment, skip the whole line
        if (!line.empty() && line.at(0) != '#')
        {
            auto words_begin = sregex_iterator(line.begin(), line.end(), words_regex);

            auto words_end = sregex_iterator();

            for (sregex_iterator k = words_begin; k != words_end; ++k)
            {
                smatch match = *k;
                string match_str = match.str();
                trim(match_str);

                if (match_str.at(0) == '(')
                {
                    openBraketNum++;
                }
                else if (match_str.at(0) == ')')
                {
                    closeBraketNum++;
                }
                else if (match_str.at(0) == '[')
                {
                    openSqBraketNum++;
                }
                else if (match_str.at(0) == ']')
                {
                    closeSqBraketNum++;
                }
            }
        }
    }
    
    // If number of open and close brackets doesn't match there's an 
    // error
    if ((openBraketNum != closeBraketNum) ||
        (openSqBraketNum != closeSqBraketNum))
    {
        noError = false;
    }

    return noError;
}

void handleAssignmentStmt(string str)
{
    string resultStr;
    string tempStr;
    int parserType = -1;

    string::size_type pos = str.find(eqStr);

    // First parse out everything on the right and left of the = sign
    string strBeforeEq = str.substr(0, pos);
    string strAfterEq = str.substr(pos + 1, str.length());

    if (debugPrint)
    {
        cout << "str: " << str << endl;
        cout << "strBeforeEq: " << strBeforeEq << endl;
        cout << "strAfterEq: " << strAfterEq << endl;
    }

    // First thing to check is whether the expression contains string
    if ((strAfterEq.find(quoteStr) != string::npos) && (strAfterEq.find(openSqBracketStr) == string::npos))
    {
        // It contains a string and not an element of strings, parse it as a string
        resultStr = lexParserForStr(strBeforeEq, strAfterEq);
    }
    else if ((strAfterEq.find(openSqBracketStr) != string::npos) && 
            ((strAfterEq.find(addOpStr) != string::npos) || (strAfterEq.find(subtractOpStr) != string::npos)))
    {
        // This contains a list addition of some sort
        // Parse this as a list
        resultStr = mathOpForList(strBeforeEq, strAfterEq);

        if (resultStr.length() != 0)
        {
            // An error returned, display it.
            cout << resultStr << endl;
        }

    }
    else if ((strAfterEq.find(openSqBracketStr) != string::npos) && (strAfterEq.find(addOpStr) == string::npos))
    {
        // It contains a open square bracket, parse if as a list
        resultStr = lexParserForList(strBeforeEq, strAfterEq);

        if (resultStr.length() != 0)
        {
            // An error is returned. Print it out
            // cout << resultStr << endl;
        }
    }
    /*
    else if ((strAfterEq.find(openSqBracketStr) != string::npos) && (strAfterEq.find(addOpStr) == string::npos))
    {
        // It contains a open square bracket, parse if as a list
        resultStr = lexParserForList(strBeforeEq, strAfterEq);

        if (resultStr.length() != 0)
        {
            // An error is returned. Print it out
            // cout << resultStr << endl;
        }
    }
    else if ((strAfterEq.find(openSqBracketStr) != string::npos) && (strAfterEq.find(addOpStr) != string::npos))
    {
        // This contains a list addition of some sort
        // Parse this as a list
        resultStr = mathOpForList(strBeforeEq, strAfterEq);

        if (resultStr.length() != 0)
        {
            // An error returned, display it.
            cout << resultStr << endl;
        }

    }
    */
    else
    {
        // This may be an operation involving an integer or a variable representing a list element. 
        // Need to find out first
        parserType = setParserType(strAfterEq);

        if (parserType == parserType_Int)
        {
            // Following are used to remove all spaces and tabs on either side of the assignment string
            strBeforeEq.erase(remove(strBeforeEq.begin(), strBeforeEq.end(), ' '), strBeforeEq.end());
            strBeforeEq.erase(remove(strBeforeEq.begin(), strBeforeEq.end(), '\t'), strBeforeEq.end());
            strAfterEq.erase(remove(strAfterEq.begin(), strAfterEq.end(), ' '), strAfterEq.end());
            strAfterEq.erase(remove(strAfterEq.begin(), strAfterEq.end(), '\t'), strAfterEq.end());
            // Handle expression on right side of '=' as an integer
            if (isNumber(strAfterEq))
            {
                // Check if strAfterEq is a number, if so add it to map
                // table.insert(pair<string, string>(strBeforeEq, strAfterEq));
                table[strBeforeEq] = strAfterEq;
            }
            else
            {
                // Check if after string has any arithematic operators
                // If so calculate them before doing assignment
                if ((strAfterEq.find(addOpStr) != string::npos) ||
                    (strAfterEq.find(subtractOpStr) != string::npos) ||
                    (strAfterEq.find(multiplyOpStr) != string::npos) ||
                    (strAfterEq.find(divideOpStr) != string::npos))
                {
                    strAfterEq = lexParserForInt(strAfterEq);
                    // table.insert(pair<string, string>(strBeforeEq, strAfterEq));
                    table[strBeforeEq] = strAfterEq;
                    resultStr = lexParserForInt(strAfterEq);
                }
            }
        }
        else if (parserType == parserType_List)
        {
            // Parse this as a list
            resultStr = mathOpForList(strBeforeEq, strAfterEq);

            if (resultStr.length() != 0)
            {
                // An error returned, display it.
                cout << resultStr << endl;
            }
        }
    }
    return;
}

int setParserType(string str)
{
    int retCode = parserType_Int;
    map<string, vector<int>>::iterator itr;
    map<string, vector<string>>::iterator itr1;

    // Check if the first string is stored in dynTable, if so
    // parse this as a list. Otherwise parse this as an integer
    regex words_regex(regstrg);

    auto words_begin = sregex_iterator(str.begin(), str.end(), words_regex);

    auto words_end = sregex_iterator();

    for (sregex_iterator k = words_begin; k != words_end; ++k)
    {
        smatch match = *k;
        string match_str = match.str();
        trim(match_str);

        // Find it first from vector int map. If not found 
        // try to find it from vector string map
        itr = dynTable.find(match_str);

        if (itr != dynTable.end())
        {
            retCode = parserType_List;
            break;
        }
        else
        {
            itr1 = dynStrTable.find(match_str);

            if (itr1 != dynStrTable.end())
            {
                retCode = parserType_List;
                break;
            }
        }

    }
    return retCode;
}

string mathOpForList(string varName, string str)
{
    bool addList = false;
    int intArraySize = 0;
    int cnt = 0;
    string retStr = "";
    vector<int> iV;
    vector<int> resVec;
    vector<string> iVStr;
    vector<string> resVecStr;
    map<string, vector<int>>::iterator itr;
    map<string, vector<int>>::iterator itr1;
    map<string, vector<string>>::iterator itrStr;
    bool hasSingleElement = false;
    bool hasSlice = false;
    string prevToken;
    int pos = 0;
    int idxPos;
    int curVarNum = 0;
    int curVarIdx = 0;
    int retrievedInt = 0;
    int retrievedSum = 0;
    string retrievedSumStr;
    int valType = parserType_Int;
    int lastVarPos = 0;
    string lastVar;
    bool lastVarIsList = false;
    char lastOperation = '+';

    // First determine where passed in str is for what case:
    // 1. Addition of two lists
    // 2. Addition of lists including single element (includes "[")
    // 3. Addition of lists include list slicing (include "[:")
    /*
    if ((str.find(openSqBracketStr) != string::npos) && (str.find(sliceStr) == string::npos))
    {
        hasSingleElement = true;
    }

    if (str.find(sliceStr) != string::npos)
    {
        hasSlice = true;
    }
    */
    if (str.find(openSqBracketStr) != string::npos)
    {
        hasSingleElement = true;
    }

    if (str.find(sliceStr) != string::npos)
    {
        hasSlice = true;
    }

    regex words_regex(regstrg);

    auto words_begin = sregex_iterator(str.begin(), str.end(), words_regex);

    auto words_end = sregex_iterator();

    for (sregex_iterator k = words_begin; k != words_end; ++k)
    {
        smatch match = *k;
        string match_str = match.str();
        trim(match_str);

        if (debugPrint) cout << match_str << endl;

        // if ((match_str.at(0) == '-') ||
        if ((match_str.at(0) == '*') ||
            (match_str.at(0) == '/'))
        {            
            {
                // return error string
                retStr = "Unsupported operand type(s) for list";
                break;
            }
        }
        else
        {
            if (hasSingleElement == false)
            {
                // Here's the list values that need handling
                // First find out the values from map dynTable
                itr = dynTable.find(match_str);

                if (itr != dynTable.end())
                {
                    iV = itr->second;

                    // Now push each integer to the resultant vector
                    for (auto i = iV.begin(); i != iV.end(); i++)
                    {
                        resVec.push_back(*i);
                    }
                }
                else
                {
                    // Can't find the values from int vector table. Try
                    // finding them from string vector table
                    itrStr = dynStrTable.find(match_str);

                    if (itrStr != dynStrTable.end())
                    {
                        iVStr = itrStr->second;

                        // Now push each integer to the resultant vector
                        for (auto i = iVStr.begin(); i != iVStr.end(); i++)
                        {
                            resVecStr.push_back(*i);
                        }
                    }
                }
            }
            else
            {
                // Involve list element or even slicing
                if (hasSlice == true)
                {
                    if (pos == 0) 
                    {
                        // First position is name of the list. Look up its integer vector from dynamic table
                        itr = dynTable.find(match_str);

                        if (itr != dynTable.end())
                        {
                            iV = itr->second;
                        }
                    }
                    else if (pos == 2)
                    {
                        // Second position indicates start index of slice
                        idxPos = stoi(match_str);
                    }
                    else if (pos == 3)
                    {
                        // Now we know what elements in the slice to include
                        for (int i = idxPos; i < iV.size(); i++)
                        {
                            retrievedInt = iV[i];
                            resVec.push_back(retrievedInt);
                        }
                    }
                    else if (pos == 6)
                    {
                        // Since addList makes pos skips 1 step
                        itr = dynTable.find(match_str);

                        if (itr != dynTable.end())
                        {
                            iV = itr->second;

                            // Now push each integer to the resultant vector
                            for (auto i = iV.begin(); i != iV.end(); i++)
                            {
                                resVec.push_back(*i);
                            }
                        }
                    }

                    pos++;
                }
                else
                {
                    if ((match_str.find(openSqBracketStr) == string::npos) &&
                        match_str.find(closeSqBracketStr) == string::npos)
                    {
                        // No slicing, all are single elements. They can also be pure numbers,
                        // or an element in map table
                        itr = dynTable.find(match_str);

                        if (itr != dynTable.end())
                        {
                            iV = itr->second;
                            lastVar = match_str;
                            lastVarPos = pos;
                            lastVarIsList = true;
                        }
                        else
                        {
                            // Can't find from vector int table. Try to find it from vector 
                            // string table
                            itrStr = dynStrTable.find(match_str);

                            if (itrStr != dynStrTable.end())
                            {
                                iVStr = itrStr->second;

                                valType = parserType_List;
                                lastVar = match_str;
                                lastVarPos = pos;
                                lastVarIsList = true;
                            }
                            else
                            {
                                // It's not really a list element, see if it's a number
                                if ((isNumber(match_str) == true) && (lastVarIsList != true))
                                {
                                    if (lastOperation == '+')
                                    {
                                        retrievedSum += stoi(match_str);
                                    }
                                    else if (lastOperation == '-')
                                    {
                                        retrievedSum -= stoi(match_str);
                                    }
                                }
                                else
                                {
                                    // It's not a number, see if it's a variable on map table
                                    if (table[match_str].length() > 0)
                                    {
                                        if (lastOperation == '+')
                                        {
                                            retrievedSum += stoi(table[match_str]);
                                        }
                                        else if (lastOperation == '-')
                                        {
                                            retrievedSum -= stoi(table[match_str]);
                                        }
                                    }
                                }
                            }
                        }


                        if (pos > 0)
                        {
                            if (pos == lastVarPos + 2)
                            {
                                if (valType == parserType_List)
                                {
                                    itrStr = dynStrTable.find(lastVar);

                                    if (itrStr != dynStrTable.end())
                                    {
                                        iVStr = itrStr->second;
                                    }

                                    retrievedSumStr += iVStr.at(stoi(match_str));

                                }
                                else
                                {
                                    idxPos = stoi(match_str);
                                    itr = dynTable.find(match_str);

                                    if (itr != dynTable.end())
                                    {
                                        iV = itr->second;
                                    }

                                    if (lastOperation == '+')
                                    {
                                        retrievedSum += iV.at(idxPos);
                                    }
                                    else if (lastOperation == '-')
                                    {
                                        retrievedSum -= iV.at(idxPos);
                                    }
                                }
                                lastVarPos = 0;
                                lastVarIsList = false;
                            }
                        }                        
                    }
                    
                    if ((match_str.at(0) == '+') || (match_str.at(0) == '-'))
                    {
                        lastOperation = match_str.at(0);
                    }
                    pos++;                    
                }
            }
        }
    }

    if (retrievedSum != 0)
    {
        resVec.push_back(retrievedSum);
    }

    if (resVec.size() > 0)
    {
        // Set value of varName as the resultant vector
        dynTable[varName] = resVec;
    }

    if (retrievedSumStr.length() > 0)
    {
        resVecStr.push_back(retrievedSumStr);
    }

    if (resVecStr.size() > 0)
    {
        // Set value of varName as the resultant vector
        dynStrTable[varName] = resVecStr;
    }

    return retStr;
}

void handlePrintStmt(string str)
{
    int pos = 0;
    map<string, string>::iterator itr;
    map<string, vector<int>>::iterator itr1;
    map<string, vector<string>>::iterator itr2;
    vector<int> iV1;
    vector<string> iV2;
    string outStr;
    int startPos = 0;
    int endPos = 0;
    string strLiteral;

    // First check whether this is just a simple string to be printed
    if (str.find("print(\"") != string::npos) 
    {
        // Extract everything between the quotes and print the string out
        startPos = str.find("(\"");

        if (startPos != string::npos)
        {
            // Skip open parenthesis and first "
            startPos += 2; 

            // Look for closing quote;
            endPos = str.find("\")");
            if (endPos != string::npos)
            {
                strLiteral = str.substr(startPos, endPos-startPos);

                if (strLiteral.length() > 0)
                {
                    cout << strLiteral << endl;
                }
            }
        }
    }
    else
    {
        regex words_regex(regstrg);

        auto words_begin = sregex_iterator(str.begin(), str.end(), words_regex);

        auto words_end = sregex_iterator();

        for (sregex_iterator k = words_begin; k != words_end; ++k)
        {
            smatch match = *k;
            string match_str = match.str();
            trim(match_str);

            // Only need to find out the parameter after the '('
            // Mismatched parathensis is caught by pre-processor
            if ((match_str.compare(printStr) != 0) &&
                (match_str.at(0) != '(') &&
                (match_str.at(0) != ')'))
            {
                // Find it from map
                itr = table.find(match_str);

                if (itr != table.end())
                {
                    // Only print value
                    cout << itr->second << endl;
                }
                else
                {
                    // Didn't find the key from the dynamic integer table. Now try
                    // to find it from the dynamic vector table
                    itr1 = dynTable.find(match_str);

                    if (itr1 != dynTable.end())
                    {
                        iV1 = itr1->second;

                        // Check if the size of the vector has one or multiple elements
                        if (iV1.size() > 1)
                        {
                            pos = 0;
                            cout << "[";
                            for (auto m = iV1.begin(); m != iV1.end(); m++)
                            {
                                if (pos < iV1.size() - 1)
                                {
                                    // Only append comma if this is not the last element in the list
                                    cout << to_string(*m) << ", ";
                                }
                                else
                                {
                                    cout << to_string(*m);
                                }
                                pos++;
                            }
                            cout << "]" << endl;
                        }
                        else
                        {
                            cout << iV1.at(0) << endl;
                        }
                    }
                    else
                    {
                        itr2 = dynStrTable.find(match_str);

                        if (itr2 != dynStrTable.end())
                        {
                            iV2 = itr2->second;

                            // Check if the size of the vector has one or multiple elements
                            if (iV2.size() > 1)
                            {
                                pos = 0;
                                cout << "[";
                                for (auto m = iV2.begin(); m != iV2.end(); m++)
                                {
                                    (*m).erase(remove((*m).begin(), (*m).end(), '\"'), (*m).end());

                                    if (pos < iV2.size() - 1)
                                    {
                                        // Only append comma if this is not the last element in the list
                                        cout << "'" << *m << "'" << ", ";
                                    }
                                    else
                                    {
                                        cout << "'" << *m << "'";
                                    }
                                    pos++;
                                }
                                cout << "]" << endl;
                            }
                            else
                            {
                                outStr = iV2.at(0);

                                // Remove all double quotes
                                outStr.erase(remove(outStr.begin(), outStr.end(), '\"'), outStr.end());
                                cout << outStr << endl;
                            }
                        }
                        else
                        {
                            // Couldn't find in either table. Display error
                            cout << "NameError: name " << match_str << " is not defined" << endl;
                        }
                    }
                }
            }
        }
    }
    return;
}

string lexParserForStr(string varName, string strg)
{
    string resultStr;

    // First is to check whether there's a + sign for string concatenation
    if (strg.find(addOpStr) == string::npos)
    {
        // '+' not found, this is just a string
        strg.erase(std::remove(strg.begin(), strg.end(), '"'), strg.end());

        // Erase whitespace on varName before assignment. This is to cope with
        // if/else case where there's a space or tab prepended
        varName.erase(std::remove(varName.begin(), varName.end(), ' '), varName.end());
        varName.erase(std::remove(varName.begin(), varName.end(), '\t'), varName.end());
        // This is a simple string assignment, put the string variable on map
        table[varName] = strg;
    }
    else
    {
        string regstrgStr = "[^\\s\\(\\),+\\-*/=:]+|[\\s+]+|[()]+|[,+\\-*/=:]";

        regex words_regex(regstrgStr);

        auto words_begin = sregex_iterator(strg.begin(), strg.end(), words_regex);

        auto words_end = sregex_iterator();
        map<string, string>::iterator itr;

        for (sregex_iterator k = words_begin; k != words_end; ++k)
        {
            smatch match = *k;
            string match_str = match.str();
            // trim(match_str);

            if (debugPrint) cout << match_str << " length: " << match_str.length() << endl;
            // cout << match_str << " length: " << match_str.length() << endl;

            if (((match_str.at(0) != '+') &&
                (match_str.at(0) != '"')) ||
                (match_str.length() > 1)) // Check for length > 1 is needed for quoted char such as "I"
            {
                // Skip + sign, concatenate the rest
                // See if this string is another variable stored in map
                itr = table.find(match_str);

                if (itr != table.end())
                {
                    resultStr += itr->second;
                }
                else
                {
                    // Check if white space need to be appended
                    resultStr += match_str;
                }

                // Remove quotes that are not kept by Python
                resultStr.erase(remove(resultStr.begin(), resultStr.end(), '\"'), resultStr.end());
            }
        }

        // Now store the resultStr on map
        // table.insert(pair<string, string>(varName, resultStr));
        table[varName] = resultStr;                
    }
    return resultStr;

}
string lexParserForInt(string strg)
{
    int val1 = 0;
    int val2 = 0;
    int temp = 0;
    map<string, string>::iterator itr;

    regex words_regex(regstrg);

    auto words_begin = sregex_iterator(strg.begin(), strg.end(), words_regex);

    auto words_end = sregex_iterator();

    for (sregex_iterator k = words_begin; k != words_end; ++k)
    {
        smatch match = *k;
        string match_str = match.str();
        trim(match_str);

        if (debugPrint) cout << match_str << " length: " << match_str.length() << endl;
        
        if (match_str.at(0) == '(')
        {
            ops.push('(');
        }
        else if (isNumber(match_str))
        {
            values.push(stoi(match_str));
        }
        else if (match_str.at(0) == ')')
        {
            while (!ops.empty() && ops.top() != '(')
            {
                val2 = values.top();
                values.pop();

                val1 = values.top();
                values.pop();

                char op = ops.top();
                ops.pop();

                values.push(mathOp(val1, val2, op));
            }

            // pop opening brace.
            if (!ops.empty())
               ops.pop();
        }
        else if (match_str.at(0) == '+' ||
                match_str.at(0) == '-' ||
                match_str.at(0) == '*' ||
                match_str.at(0) == '/')
        {
            // This is an operator
            while (!ops.empty() && precedence(ops.top()) >= precedence(match_str.at(0)))
            {
                val2 = values.top();
                values.pop();

                val1 = values.top();
                values.pop();

                char op = ops.top();
                ops.pop();

                values.push(mathOp(val1, val2, op));
            }

            // Push current token to 'ops'.
            ops.push(match_str.at(0));
        }
        else
        {
            // This is where a previously assigned variable 
            // will be handled
            itr = table.find(match_str);
            if (debugPrint) cout << "Key: " << itr->first << ", value: " << itr->second << endl;

            if (regex_match(itr->second, integer_expr))
            {
                temp = stoi(itr->second);
            }

            values.push(temp);

        }
    }

    // Now pop everything and handle them
    while (!ops.empty()) {
        val2 = values.top();
        values.pop();

        val1 = values.top();
        values.pop();

        char op = ops.top();
        ops.pop();

        values.push(mathOp(val1, val2, op));
    }

    // Result is at the top
    return to_string(values.top());
}

string lexParserForList(string varName, string strg)
{
    string retStr = "";
    int parseType = -1;
    bool allProcessed = false;
    vector<int> iV;
    vector<int> iV1;
    vector<string> iVs;
    map<string, vector<int>>::iterator itr;
    int arraySize = 0;
    int pos;
    int idxPos;
    int retrievedInt = 0;

    regex words_regex(regstrg);

    auto words_begin = sregex_iterator(strg.begin(), strg.end(), words_regex);

    auto words_end = sregex_iterator();

    // There are 2 cases to process. Either the list has only 1 element, or it has multiple
    // elements. In the latter case it's separated by a comma. So first thing is to see if there's a
    // comma
    if (strg.find(commaStr) != string::npos)
    {
        for (sregex_iterator k = words_begin; k != words_end; ++k)
        {
            smatch match = *k;
            string match_str = match.str();
            trim(match_str);

            if (debugPrint) cout << match_str << " length: " << match_str.length() << endl;

            if ((match_str.at(0) != '[') &&
                (match_str.at(0) != ','))
            {
                // This is value inside list, but before end of the list, check if it's an integer or a string           
                if (isNumber(match_str))
                {
                    listIntVals.push(stoi(match_str));
                    parseType = parserType_Int;
                }
                else if (match_str.find(quoteStr) != string::npos)
                {
                    // This is a list with string elements
                    listStrVals.push(match_str);
                }
                else if ((match_str.at(0) == ']'))
                {
                    // End of list values
                    allProcessed = true;
                }
            }

            if (allProcessed)
            {
                if (parseType == parserType_Int)
                {
                    // List values are integers, create an integer array to store the integers, and insert 
                    // the integer array to the dynamic map for integer
                    arraySize = listIntVals.size();
                    if (arraySize > 0)
                    {
                        for (int i = 0; i < arraySize; i++)
                        {
                            iV.push_back(listIntVals.front());
                            listIntVals.pop();
                        }
                    }
                    // Now set the key varName with the populated integer array
                    dynTable[varName] = iV;
                }
                else
                {
                    arraySize = listStrVals.size();
                    if (arraySize > 0)
                    {
                        for (int i = 0; i < arraySize; i++)
                        {
                            iVs.push_back(listStrVals.front());
                            listStrVals.pop();
                        }
                    }
                    // Now set the key varName with the populated integer array
                    dynStrTable[varName] = iVs;
                }
            }
        }
    }
    else
    {
        pos = 0;
        // No comma, just assign 1 list element
        for (sregex_iterator k = words_begin; k != words_end; ++k)
        {
            smatch match = *k;
            string match_str = match.str();
            trim(match_str);

            if (pos == 0)
            {
                // First position is name of the list. Look up its integer vector from dynamic table
                itr = dynTable.find(match_str);

                if (itr != dynTable.end())
                {
                    // iV will be used to find the specific index later on
                    iV = itr->second;
                }
            }
            else if (pos == 2)
            {                
                // This is the index of the vector to be retrieved
                idxPos = stoi(match_str);

                if (idxPos < iV.size())
                {
                    retrievedInt = iV[idxPos];
                    if (debugPrint) cout << "Retrieved int: " << to_string(retrievedInt) << endl;
                }
                else
                {
                    retStr = "IndexError: list index out of range";
                }
            }
            if (debugPrint) cout << match_str << " length: " << match_str.length() << endl;

            pos++;
        }

        // If there's no error then store the retrieved integer to a vector of integer before pushing it back
        // to dynamic table
        if (retStr.length() == 0)
        {
            iV1.push_back(retrievedInt);

            dynTable[varName] = iV1;
        }
    }

    return retStr;
}

int precedence(char op) 
{
    if (op == '+' || op == '-')
        return 1;
    if (op == '*' || op == '/')
        return 2;
    return 0;
}

int mathOp(int a, int b, char op) 
{
    switch (op) 
    {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return a / b;
    }

    return 0;
}

bool isNumber(const string& str)
{
    for (char const& c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}

string trim(string s) 
{
    regex e("^\\s+|\\s+$");  
    return regex_replace(s, e, "");
}

void handleLambdaStmt(string str)
{
    cout << "To be developed..." << endl;

    return;
}