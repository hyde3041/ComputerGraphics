#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace std;

const int dataSize{ 10 };

typedef class data {
public:
	bool exist{ false };
	int xPos{};
	int yPos{};
	int zPos{};
} Data;

typedef class DataLenth {
public:
	bool exist{ false };
	int num{};
	int lenth{};
} dataLenth;

typedef class AllLenth {
public:
	int first{};
	int second{};
	int lenth{};
} allLenth;

Data list[dataSize];
dataLenth listLenth[dataSize];

char command;

bool fFlag{ false };


// 거리 리스트 최신화
void updateLenth(Data* list) {

	for (int i = 0; i < dataSize; ++i) {
		listLenth[i] = dataLenth{};
	}

	for (int i = 0; i < dataSize; ++i) {

		if (list[i].exist) {

			listLenth[i].exist = true;
			listLenth[i].num = i;

			listLenth[i].lenth = static_cast<int>(
				sqrt(
					list[i].xPos * list[i].xPos +
					list[i].yPos * list[i].yPos +
					list[i].zPos * list[i].zPos
				)
				);
		}
	}

	sort(
		listLenth,
		listLenth + dataSize,

		[](const dataLenth& left, const dataLenth& right) {

			if (left.exist != right.exist) {
				return left.exist > right.exist;
			}

			return left.lenth < right.lenth;
		}
	);
}


// 리스트 출력
void printList(Data* list) {

	if (fFlag) {

		// f 모드일 때 항상 최신화
		updateLenth(list);

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
			else {
				cout << i << ": 없음\n";
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
			else {

				cout << i << ": 없음\n";
			}
		}
	}
}


// 위쪽 추가
void makeTop(Data* list) {

	Data temp;

	if (list[dataSize - 1].exist) {

		cout << "추가가 불가능합니다.\n";

		return;
	}

	cout << "숫자를 입력해 주세요\n";

	if (!(cin >> temp.xPos >> temp.yPos >> temp.zPos)) {

		cout << "숫자만 입력해주세요.\n";

		cin.clear();

		cin.ignore(
			numeric_limits<streamsize>::max(),
			'\n'
		);

		return;
	}

	temp.exist = true;

	for (int i = 0; i < dataSize; ++i) {

		if (!list[i].exist) {

			list[i] = temp;

			break;
		}
	}

	printList(list);
}


// 위쪽 삭제
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


// 아래쪽 추가
void makeDown(Data* list) {

	if (
		list[0].exist == true &&
		list[dataSize - 1].exist == true
		) {

		cout << "추가가 불가능합니다\n";

		return;
	}

	Data temp;

	cout << "숫자를 입력해 주세요\n";

	if (!(cin >> temp.xPos >> temp.yPos >> temp.zPos)) {

		cout << "숫자만 입력해주세요.\n";

		cin.clear();

		cin.ignore(
			numeric_limits<streamsize>::max(),
			'\n'
		);

		return;
	}

	temp.exist = true;


	if (list[0].exist) {

		for (int i = dataSize - 2; i > -1; --i) {

			list[i + 1] = list[i];
		}
	}


	for (int i = 0; i < dataSize - 1; ++i) {

		if (list[i + 1].exist) {

			list[i] = temp;

			printList(list);

			return;
		}
	}


	list[0] = temp;

	printList(list);
}


// 아래쪽 삭제
void deletDown(Data* list) {

	for (int i = 0; i < dataSize; ++i) {

		if (list[i].exist) {

			list[i] = Data{};

			break;
		}
	}

	printList(list);
}


// 데이터 개수
void cntList(Data* list) {

	int cnt{ 0 };

	for (int i = 0; i < dataSize; ++i) {

		if (list[i].exist) {

			++cnt;
		}
	}

	cout << cnt << "개\n";
}


// 한 칸 이동
void moveList(Data* list) {

	Data temp;

	temp = list[0];

	for (int i = 1; i < dataSize; ++i) {

		list[i - 1] = list[i];
	}

	list[dataSize - 1] = temp;

	printList(list);
}


// 전체 삭제
void deletList(Data* list) {

	for (int i = 0; i < dataSize; ++i) {

		list[i] = Data{};
	}

	printList(list);
}


// 거리순 모드
void orgLenth(Data* list) {

	fFlag = !fFlag;

	printList(list);
}


// 가장 가까운 좌표 / 가장 먼 좌표
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


			lengths.push_back({
				i,
				j,
				distance
				});
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
		<< "거리: "
		<< nearest.lenth << '\n';


	cout << "가장 먼 좌표: "
		<< farthest.first << "번과 "
		<< farthest.second << "번\n"
		<< "거리: "
		<< farthest.lenth << '\n';
}


int main()
{
	while (true) {

		cout << "\n명령어를 입력해주세요: ";

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

	return 0;
}