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

// e에서 각 행마다 뺀 값 저장
int firstE[maxNum];
int secoundE[maxNum];

// f에서 각 행마다 더한 값 저장
int firstF[maxNum];
int secoundF[maxNum];


void makeNumber(int number[][maxNum]) {
	for (int i = 0; i < maxNum; ++i) {
		for (int j = 0; j < maxNum; ++j) {
			number[i][j] = rand() % 10;
		}
	}
}


void printNumber(int number[][maxNum]) {
	for (int i = 0; i < maxNum; ++i) {
		for (int j = 0; j < maxNum; ++j) {
			cout << number[i][j] << " ";
		}

		cout << "\n";
	}
}


// 행렬 곱
// 원본 수정 X
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


// 행렬 덧셈
// 원본 수정 X
void addNumber(int first[][maxNum], int secound[][maxNum]) {
	int third[maxNum][maxNum];

	for (int i = 0; i < maxNum; ++i) {
		for (int j = 0; j < maxNum; ++j) {
			third[i][j] = first[i][j] + secound[i][j];
		}
	}

	printNumber(third);
}


// 행렬 뺄셈
// 원본 수정 X
void minusNumber(int first[][maxNum], int secound[][maxNum]) {
	int third[maxNum][maxNum];

	for (int i = 0; i < maxNum; ++i) {
		for (int j = 0; j < maxNum; ++j) {
			third[i][j] = first[i][j] - secound[i][j];
		}
	}

	printNumber(third);
}


// 3x3 행렬식
int detNum(int number[][maxNum - 1]) {
	int addSum1 = 1;
	int addSum2 = 0;

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			int k = j + i;

			if (k >= 3) {
				k -= 3;
			}

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

			if (k < 0) {
				k += 3;
			}

			minusSum1 *= number[k][j];
		}

		minusSum2 += minusSum1;
		minusSum1 = 1;
	}

	return addSum2 - minusSum2;
}


// 4x4 행렬식
void detNumber(int number[][maxNum]) {
	int temp[3][3];
	int result = 0;

	for (int removeCol = 0; removeCol < maxNum; ++removeCol) {
		int count = 0;

		for (int row = 1; row < maxNum; ++row) {
			for (int col = 0; col < maxNum; ++col) {
				if (col == removeCol) {
					continue;
				}

				temp[count / 3][count % 3] = number[row][col];
				++count;
			}
		}

		if (removeCol % 2 == 0) {
			result += number[0][removeCol] * detNum(temp);
		}
		else {
			result -= number[0][removeCol] * detNum(temp);
		}
	}

	cout << result << "\n";
}


// 전치행렬
// 원본 수정
void revNumber(int number[][maxNum]) {
	for (int i = 0; i < maxNum; ++i) {
		for (int j = i + 1; j < maxNum; ++j) {
			int temp = number[i][j];

			number[i][j] = number[j][i];
			number[j][i] = temp;
		}
	}
}


// e
// ON  : 각 행의 최솟값을 빼고 저장
// OFF : 저장한 값을 다시 더함
void eNumber(int number[][maxNum], int save[]) {
	if (!eFlag) {
		for (int i = 0; i < maxNum; ++i) {
			int temp = number[i][0];

			// 최솟값 찾기
			for (int j = 0; j < maxNum; ++j) {
				if (temp > number[i][j]) {
					temp = number[i][j];
				}
			}

			// 얼마를 뺐는지 저장
			save[i] = temp;

			// 최솟값 빼기
			for (int j = 0; j < maxNum; ++j) {
				number[i][j] -= temp;
			}
		}
	}
	else {
		// e 해제
		for (int i = 0; i < maxNum; ++i) {
			for (int j = 0; j < maxNum; ++j) {
				number[i][j] += save[i];
			}
		}
	}
}


// f
// ON  : 각 행의 최댓값을 더하고 저장
// OFF : 저장한 값을 다시 뺌
void fNumber(int number[][maxNum], int save[]) {
	if (!fFlag) {
		for (int j = 0; j < maxNum; ++j) {
			int temp = number[0][j];

			// 각 열의 최댓값 찾기
			for (int i = 0; i < maxNum; ++i) {
				if (temp < number[i][j]) {
					temp = number[i][j];
				}
			}

			save[j] = temp;

			// 해당 열 전체에 최댓값 더하기
			for (int i = 0; i < maxNum; ++i) {
				number[i][j] += temp;
			}
		}
	}
	else {
		for (int j = 0; j < maxNum; ++j) {
			for (int i = 0; i < maxNum; ++i) {
				number[i][j] -= save[j];
			}
		}
	}
}


// 모든 값 +1
// 현재 배열 기준
void plusOne(int number[][maxNum]) {
	for (int i = 0; i < maxNum; ++i) {
		for (int j = 0; j < maxNum; ++j) {
			++number[i][j];

			number[i][j] %= 10;
		}
	}
}


// 모든 값 -1
// 현재 배열 기준
void minusOne(int number[][maxNum]) {
	for (int i = 0; i < maxNum; ++i) {
		for (int j = 0; j < maxNum; ++j) {
			--number[i][j];

			if (number[i][j] < 0) {
				number[i][j] = 9;
			}
		}
	}
}


int main()
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

		// 곱셈
		if (command == 'm') {
			multNumber(first, secound);
		}

		// 덧셈
		else if (command == 'a') {
			addNumber(first, secound);
		}

		// 뺄셈
		else if (command == 'd') {
			minusNumber(first, secound);
		}

		// 행렬식
		else if (command == 'r') {
			cout << "first\n";
			detNumber(first);

			cout << "\nsecound\n";
			detNumber(secound);
		}

		// 전치
		else if (command == 't') {
			revNumber(first);
			revNumber(secound);

			cout << "first\n";
			printNumber(first);
			cout << "행렬식: ";
			detNumber(first);

			cout << "\nsecound\n";
			printNumber(secound);
			cout << "행렬식: ";
			detNumber(secound);
		}

		// e 토글
		else if (command == 'e') {
			eNumber(first, firstE);
			eNumber(secound, secoundE);

			eFlag = !eFlag;

			cout << "first\n";
			printNumber(first);

			cout << "\nsecound\n";
			printNumber(secound);
		}

		// f 토글
		else if (command == 'f') {
			fNumber(first, firstF);
			fNumber(secound, secoundF);

			fFlag = !fFlag;

			cout << "first\n";
			printNumber(first);

			cout << "\nsecound\n";
			printNumber(secound);
		}

		// +1
		else if (command == '+') {
			plusOne(first);
			plusOne(secound);

			cout << "first\n";
			printNumber(first);

			cout << "\nsecound\n";
			printNumber(secound);
		}

		// -1
		else if (command == '-') {
			minusOne(first);
			minusOne(secound);

			cout << "first\n";
			printNumber(first);

			cout << "\nsecound\n";
			printNumber(secound);
		}

		// 새 랜덤 생성
		else if (command == 's') {
			makeNumber(first);
			makeNumber(secound);

			eFlag = false;
			fFlag = false;

			cout << "first\n";
			printNumber(first);

			cout << "\nsecound\n";
			printNumber(secound);
		}

		// 종료
		else if (command == 'q') {
			break;
		}
	}

	return 0;
}