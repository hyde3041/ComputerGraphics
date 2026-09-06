#include<iostream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

const int dataSize{10};

typedef class data {
public:
	bool exist{ false };
	int xPos{};
	int yPos{};
	int zPos{};
}Data;

typedef class DataLenth {
public:
	bool exist{ false };
	int num{};
	int lenth{};
}dataLenth;

typedef class AllLenth {
public:
	int first{};
	int second{};
	int lenth{};
}allLenth;

Data list[dataSize];
dataLenth listLenth[dataSize];
char command;

bool fFlag{ false };

void printList(Data* list) {
	if (fFlag) {
		// 거리 오름차순 출력
		for (int i = 0; i < dataSize; ++i) {
			if (listLenth[i].exist) {
				int num = listLenth[i].num;

				cout << i << ": "
					<< list[num].xPos << ", "
					<< list[num].yPos << ", "
					<< list[num].zPos
					<< " / 거리: "
					<< listLenth[i].lenth << '\n';
			}
		}
	}
	else {
		for (int i = dataSize - 1; i >= 0; --i) {
			if (list[i].exist) {
				cout << i << ": "
					<< list[i].xPos << ", "
					<< list[i].yPos << ", "
					<< list[i].zPos << '\n';
			}
		}
	}
}

void makeTop(Data* list) {
	Data temp;

	// 가득 찼으면 입력받지 않고 종료
	if (list[dataSize - 1].exist) {
		cout << "추가가 불가능합니다.\n";
		return;
	}

	cout << "숫자를 입력해 주세요";
	cin >> temp.xPos >> temp.yPos >> temp.zPos;
	temp.exist = true;

	for (int i = 0; i < dataSize; ++i) {
		if (!list[i].exist) {
			list[i] = temp;
			break;
		}
	}
	printList(list);
}

void deletTop(Data* list) {
	for (int i = dataSize - 1; i >= 0; --i) {
		if (list[i].exist) {
			list[i] = Data{};
			printList(list);
			return;
		}
	}
	cout << "삭제할 데이터가 없습니다.\n";
}

void makeDown(Data* list) {

	//제일 위랑 제일아래랑 다차있는지 확인하는코드
	if (list[0].exist == true && list[dataSize - 1].exist == true) {
		cout << "추가가 불가능합니다";
		return;
	}

	Data temp;

	cout << "숫자를 입력해 주세요";
	cin >> temp.xPos >> temp.yPos >> temp.zPos;
	temp.exist = true;

	//0번 안비어 있으면 1칸씩 올리는거
	if (list[0].exist) {
		for (int i = dataSize - 2; i > -1; --i) {
			list[i + 1] = list[i];
		}
	}
		
	//제일 밑에꺼 찾아서 넣는거
	for (int i = 0; i < dataSize - 1; ++i) {
		if (list[i + 1].exist) {
			list[i] = temp;
			printList(list);
			return;
		}
	}
	//아무것도 없으면
	list[0] = temp;

	printList(list); 
}

void deletDown(Data* list) {
	for (int i = 0; i < dataSize; ++i) {
		if (list[i].exist) {
			list[i].xPos = 0;
			list[i].yPos = 0;
			list[i].zPos = 0;
			list[i].exist = false;
			break;
		}
	}
	printList(list);
}

void cntList(Data* list) {
	int cnt{ 0 };

	for (int i = 0; i < dataSize; ++i) {
		if (list[i].exist) {
			++cnt;
		}
	}

	cout << cnt << "개";
}

void moveList(Data* list) {
	Data temp;

	temp = list[0];

	for (int i = 1; i < dataSize; ++i) {
		list[i-1] = list[i];
	}

	list[9] = temp;
	printList(list);
}

void deletList(Data* list) {
	for (int i = 0; i < dataSize; ++i) {
		list[i] = Data{};
	}
	printList(list);
}

void orgLenth(Data* list) {
	fFlag = !fFlag;

	if (!fFlag) {
		printList(list);
		return;
	}

	// 이전 거리 데이터 초기화
	for (int i = 0; i < dataSize; ++i) {
		listLenth[i] = dataLenth{};
	}

	for (int i = 0; i < dataSize; ++i) {
		if (list[i].exist) {
			listLenth[i].exist = true;
			listLenth[i].num = i;
			listLenth[i].lenth = sqrt(list[i].xPos * list[i].xPos + list[i].yPos * list[i].yPos + list[i].zPos * list[i].zPos);
		}
	}

	// 10칸 전체 정렬
	sort(
		listLenth,
		listLenth + dataSize,
		[](const dataLenth& left, const dataLenth& right) {
			// 존재하는 데이터를 빈 데이터보다 앞으로 보냄
			if (left.exist != right.exist) {
				return left.exist > right.exist;
			}

			// 둘 다 존재하면 거리 오름차순
			return left.lenth < right.lenth;
		}
	);

	printList(list);
}

void bestLenth(Data* list) {
	vector<allLenth> lengths;

	for (int i = 0; i < dataSize; ++i) {
		if (!list[i].exist) {
			continue;
		}

		for (int j = i + 1; j < dataSize; ++j) {
			if (!list[j].exist) {
				continue;
			}

			int distance = static_cast<int>(
				sqrt(
					(list[i].xPos - list[j].xPos) *
					(list[i].xPos - list[j].xPos) +

					(list[i].yPos - list[j].yPos) *
					(list[i].yPos - list[j].yPos) +

					(list[i].zPos - list[j].zPos) *
					(list[i].zPos - list[j].zPos)
				)
				);

			lengths.push_back({ i, j, distance });
		}
	}

	if (lengths.empty()) {
		cout << "좌표가 2개 이상 필요합니다.\n";
		return;
	}

	allLenth nearest = lengths[0];
	allLenth farthest = lengths[0];

	for (const allLenth& item : lengths) {
		if (item.lenth < nearest.lenth) {
			nearest = item;
		}

		if (item.lenth > farthest.lenth) {
			farthest = item;
		}
	}

	cout << "가장 가까운 좌표: "
		<< nearest.first << "번과 "
		<< nearest.second << "번\n"
		<< "거리: " << nearest.lenth << '\n';

	cout << "가장 먼 좌표: "
		<< farthest.first << "번과 "
		<< farthest.second << "번\n"
		<< "거리: " << farthest.lenth << '\n';
}

void main()
{
	while (true) {
		cout << "명령어를 입력해주세요";
		cin >> command;

		if (command == '+') {
			makeTop(list);
		}
		else if (command == '-') {
			deletTop(list);
		}
		else if (command == 'e') {
			makeDown(list);
		}
		else if (command == 'd') {
			deletDown(list);
		}
		else if (command == 'a') {
			cntList(list);
		}
		else if (command == 'b') {
			moveList(list);
		}
		else if (command == 'c') {
			deletList(list);
		}
		else if (command == 'f') {
			orgLenth(list);
		}
		else if (command == 'g') {
			bestLenth(list);
		}
		else if (command == 'q') {
			break;
		}
	}
}