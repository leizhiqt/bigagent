#include <iostream>

using namespace std;

extern "C"
{
#include "string.h"
#include "ulog.h"

typedef struct _capture_file {
	int a;
	int b;
} capture_file;

}

#ifndef TEST_H
#define TEST_H

class ClassA
{

};

class ClassAA : public ClassA
{
public:
	explicit ClassAA();
	void testPrint();
private:
	capture_file *cap_file_;
	int c;
};

ClassAA::ClassAA(void) :
    cap_file_(0),
    c(9)
{
printf("ClassAA\n");
printf("5555:%p\n",cap_file_);
	//cap_file_->a=10;
	//printf("5555:%d\n",cap_file_->a);
	//fprintf(stdout, "%s %d\n",cap_file_->a);
}

void ClassAA::testPrint()
{
	printf("3333\n");
	printf("444:%d\n",cap_file_->a);
}

#endif // TEST_H

void usage(char *p)
{
	fprintf(stdout, "%s \n",p);
}
int main(int argc, char** argv,char **envp)
try
{
	printf("111\n");
	ClassAA aa;
	printf("222\n");
	aa.testPrint();
	debug("Main App exit");
}
catch(exception& e)
{
	cout << e.what() << endl;
}




