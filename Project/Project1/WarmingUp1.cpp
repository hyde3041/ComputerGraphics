#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

const int maxNum{ 4 };

int first[maxNum][maxNum];
int secound[maxNum][maxNum];
char command;

bool eFlag{ false };
bool fFlag{ false };

void makeNumber(int number[][maxNum]) {
	for (int i = 0; i < maxNum; ++i) {
		for (int j = 0; j < maxNum; ++j) {
			number[i][j] = rand() % 10;
		}
	}
}

void printNumber(int number[][maxNum]) {
	if (eFlag) {
		int temp;
		for (int i = 0; i < maxNum; ++i) {
				temp = number[i][0];
			for (int j = 0; j < maxNum; ++j) {
				if (temp > number[i][j]) {
					temp = number[i][j];
				}
			}
			for (int j = 0; j < maxNum; ++j) {
				cout << number[i][j]-temp << " ";
			}
			cout << "\n";
		}
	}
	else if (fFlag) {
		int temp;
		for (int i = 0; i < maxNum; ++i) {
			temp = number[i][0];
			for (int j = 0; j < maxNum; ++j) {
				if (temp < number[i][j]) {
					temp = number[i][j];
				}
			}
			for (int j = 0; j < maxNum; ++j) {
				cout << number[i][j] + temp << " ";
			}
			cout << "\n";
		}
	}
	else {
		for (int i = 0; i < maxNum; ++i) {
			for (int j = 0; j < maxNum; ++j) {
				cout << number[i][j] << " ";
			}
			cout << "\n";
		}
	}
}

void multNumber(int first[][maxNum], int secound[][maxNum]) {
	int third[maxNum][maxNum];

	for (int i = 0; i < maxNum; ++i) {
		for (int j = 0; j < maxNum; ++j) {
			int temp = 0;

			for (int k = 0; k < maxNum; ++k) {
				temp += first[i][k] * secound[k][j];
			}

			third[i][j] = temp;
		}
	}

	printNumber(third);
}

void addNumber(int first[][maxNum], int secound[][maxNum]) {
	int third[maxNum][maxNum];

	for (int i = 0; i < maxNum; ++i) {
		for (int j = 0; j < maxNum; ++j) {
			third[i][j] = first[i][j] + secound[i][j];
		}
	}

	printNumber(third);
}

void minusNumber(int first[][maxNum], int secound[][maxNum]) {
	int third[maxNum][maxNum];

	for (int i = 0; i < maxNum; ++i) {
		for (int j = 0; j < maxNum; ++j) {
			third[i][j] = first[i][j] - secound[i][j];
		}
	}

	printNumber(third);
}

int detNum(int number[][maxNum-1]) {
	int addSum1 = 1;
	int addSum2 = 0;

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			int k = j + i;
			if (k >= 3) k -= 3;
			addSum1 *= number[k][j];
		}
		addSum2 += addSum1;
		addSum1 = 1;
	}

	int minusSum1 = 1;
	int minusSum2 = 0;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			int k = i - j;
			if (k < 0) k += 3;
			minusSum1 *= number[k][j];
		}
		minusSum2 += minusSum1;
		minusSum1 = 1;
	}

	return (addSum2 - minusSum2);
}

void detNumber(int number[][maxNum]) {
	int temp[3][3];
	int result = 0;

	for (int removeCol = 0; removeCol < maxNum; ++removeCol) {

		int count = 0;

		for (int row = 1; row < maxNum; ++row) {
			for (int col = 0; col < maxNum; ++col) {
				if (col == removeCol) continue;

				temp[count / 3][count % 3] = number[row][col];
				count++;
			}
		}

		if (removeCol % 2 == 0)
			result += number[0][removeCol] * detNum(temp);
		else
			result -= number[0][removeCol] * detNum(temp);
	}

	cout << result << endl;
}

void revNumber(int number[][maxNum]) {
	int temp[maxNum][maxNum];
	for (int i = 0; i < maxNum; ++i) {
		for (int j = 0; j < maxNum; ++j) {
			temp[j][i] = number[i][j];
		}
	}
	printNumber(temp);
}

void plusOne(int number[][maxNum]) {
	for (int i = 0; i < maxNum; ++i) {
		for (int j = 0; j < maxNum; ++j) {
			++number[i][j];
			number[i][j] = number[i][j] % 10;
		}
	}
	printNumber(number);
}

void minusOne(int number[][maxNum]) {
	for (int i = 0; i < maxNum; ++i) {
		for (int j = 0; j < maxNum; ++j) {
			--number[i][j];
			if (number[i][j]<0) {
				number[i][j] = 9;
			}
		}
	}
	printNumber(number);
}

void main()
{
	srand((unsigned int)time(NULL));
	makeNumber(first);
	makeNumber(secound);

	cout << "first\n";
	printNumber(first);
	cout << "\nsecound\n";
	printNumber(secound);

	while (true) {
		cout << "\nCommand를 입력하시오: ";
		cin >> command;

		if (command == 'm') {
			multNumber(first, secound);
		}
		else if (command == 'a') {
			addNumber(first, secound);
		}
		else if (command == 'd') {
			minusNumber(first, secound);
		}
		else if (command == 'r') {
			cout << "first\n";
			detNumber(first);

			cout << "\nsecound\n";
			detNumber(secound);
		}
		else if (command == 't') {
			cout << "first\n";
			revNumber(first);

			cout << "\nsecound\n";
			revNumber(secound);
		}
		else if (command == 'e') {
			eFlag = !eFlag;

			cout << "first\n";
			printNumber(first);

			cout << "\nsecound\n";
			printNumber(secound);
		}
		else if (command == 'f') {
			fFlag = !fFlag;

			cout << "first\n";
			printNumber(first);

			cout << "\nsecound\n";
			printNumber(secound);
		}
		else if (command == '+') {
			cout << "first\n";
			plusOne(first);

			cout << "\nsecound\n";
			plusOne(secound);
		}
		else if (command == '-') {
			cout << "first\n";
			minusOne(first);

			cout << "\nsecound\n";
			minusOne(secound);
		}
		else if (command == 's') {
			makeNumber(first);
			makeNumber(secound);
		}
		else if (command == 'q') {
			break;
		}
	}
}