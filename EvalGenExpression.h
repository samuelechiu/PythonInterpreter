#pragma once
#include <iostream>
#include <string>
#include <regex> 
#include <stack>
#include <map>
#include <algorithm>


const string eqStr = "=";
const string ifStr = "if";
const string printStr = "print";
const string commaStr = ",";
const string sliceStr = ":";
const string addOpStr = "+";
const string subtractOpStr = "-";
const string multiplyOpStr = "*";
const string divideOpStr = "/";
const string quoteStr = "\"";
const string openSqBracketStr = "[";
const string closeSqBracketStr = "]";
const string decimalStr = ".";
const string GTStr = ">";
const string GEStr = ">=";
const string LTStr = "<";
const string LEStr = "<=";
const string lessThanStr = "<";
const string lambdaStr = "lambda";
bool debugPrint = false;

const int parserType_Int = 0;
const int parserType_List = 1;

const int cond_GT = 0;
const int cond_GE = 1;
const int cond_LT = 2;
const int cond_LE = 3;

regex integer_expr = regex("^0$|^[1-9][0-9]*$");
regex string_expr = regex("^[\"A-Za-z0-9._%-]*$");

// string regstrg = "[^\\s\\(\\),+\\-*/=:]+|[()]+|[,+\\-*/=:]";
// string regstrg = "[^\\s\\(\\)\\[\\],+\\-*/=:]+|[()]+|[\\[\\]]+|[,+\\-*/=:]";
string regstrg = "[^\\s\\(\\)\\[\\]^<=^>=,+\\-*/=:><]+|[()]+|[\\[\\]]+|[,+\\-*/=:]+|[><]";

bool errorFree(fstream &file);
void handleAssignmentStmt(string str);
void handlePrintStmt(string str);
void handleIfBlock(vector<string> ifBlockStr);
void handleLambdaStmt(string str);

string lexParserForStr(string varName, string strg);
string lexParserForInt(string strg);
string lexParserForList(string varName, string strg);
int setParserType(string strg);
int precedence(char op);
int mathOp(int a, int b, char op);
string mathOpForList(string varName, string str);
bool isNumber(const string& str);

// Helper function
string trim(string s);

// Global variables
// stack to store integer values.
stack <int> values;

// stack to store operators.
stack <char> ops;

// Queue to store list integers
queue <int> listIntVals;

// Queue to store list strings
queue <string> listStrVals;

// Map to store key value pairs. Key is always a string, value can
// be an integer or string
map<string, string> table;

// map<string, int*> dynTable;
map<string, vector<int>> dynTable;

// Map to store string to vector of strings
map<string, vector<string>> dynStrTable;
