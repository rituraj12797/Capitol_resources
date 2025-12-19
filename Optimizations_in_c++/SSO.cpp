/*
SSO - normally fr strings what used to hapen is they were allocated dynamic memory and the string class contained information like pointer to that memory in heap, and size information, but SSO  says that if a string is smal enough ( 15 16 chars ) then it can directly be stored inside an objet instead of storig in heap and savng a pointer to it.
*/

#include<bits/stdc++.h>
using namespace std;

int main() {
    string str = "somethign small";
}