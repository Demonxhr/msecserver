#ifndef BASE64_H
#define BASE64_H
#include <string>
#include <iostream>
using namespace std;
class Base64
{
public:
    Base64();
    ~Base64();
    string Base64Encode(const string &input);

    string Base64Decode(const string &input);

};

#endif