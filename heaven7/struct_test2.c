
#include <stdlib.h>
#include <stdio.h>

struct Stu{
	int age;
	char* name;
};

//c 语言没有引用. c++ 才有
//void test_ref(struct Stu& stu){
//	printf("test_ref >> name = %s, age = %d\n", stu.name, stu.age);
//}

void test_pointer(struct Stu* stu){
	printf("test_pointer = %s, age = %d\n", stu->name, stu->age);
}

void test_copy(struct Stu stu){
	printf("test_copy = %s, age = %d\n", stu.name, stu.age);
}
