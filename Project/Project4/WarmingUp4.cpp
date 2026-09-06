#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <limits>
#define NOMINMAX
#include <windows.h>

using namespace std;

const int maxSize{ 6 };

char board[maxSize][maxSize];
bool opened[maxSize][maxSize];

int width{};
int height{};
int score{};
int tryCount{};
int maxTry{};

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void makeBoard() {
    vector<char> cards;

    int boardSize = width * height;
    int pairCount = boardSize / 2;

    // 같은 소문자를 2개씩 추가
    for (int i = 0; i < pairCount; ++i) {
        char ch = 'a' + i;

        cards.push_back(ch);
        cards.push_back(ch);
    }

    // 칸 수가 홀수면 조커 추가
    if (boardSize % 2 == 1) {
        cards.push_back('@');
    }

    // 카드 무작위 섞기
    for (int i = 0; i < cards.size(); ++i) {
        int randomIndex = rand() % cards.size();

        char temp = cards[i];
        cards[i] = cards[randomIndex];
        cards[randomIndex] = temp;
    }

    // 보드에 카드 저장
    int count{};

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            board[row][col] = cards[count++];
            opened[row][col] = false;
        }
    }

    score = 0;
    tryCount = 0;
    maxTry = boardSize * 2;
}

void printBoard(bool showAll = false, int selectRow1 = -1, int selectCol1 = -1, int selectRow2 = -1, int selectCol2 = -1) {
    cout << "\n    ";

    // 위쪽 알파벳 출력
    for (int col = 0; col < width; ++col) {
        cout << static_cast<char>('a' + col) << "   ";
    }

    cout << '\n';

    for (int row = 0; row < height; ++row) {
        cout << row + 1 << "   ";

        for (int col = 0; col < width; ++col) {
            bool selected = (row == selectRow1 && col == selectCol1) || (row == selectRow2 && col == selectCol2);

            // 맞춘 카드
            if (opened[row][col]) {
                setColor(10);

                if (board[row][col] == '@') {
                    cout << '@';
                }
                else {
                    cout << static_cast<char>(toupper(static_cast<unsigned char>(board[row][col])));
                }
            }
            // 힌트 또는 현재 선택 카드
            else if (showAll || selected) {
                setColor(12);
                cout << board[row][col];
            }
            // 가려진 카드
            else {
                setColor(15);
                cout << '*';
            }

            setColor(15);
            cout << "   ";
        }

        cout << '\n';
    }

    cout << "\n점수: " << score << " / 시도: " << tryCount << " / 최대 시도: " << maxTry << '\n';
}

bool changePosition(const string& position, int& row, int& col) {
    if (position.size() != 2) {
        return false;
    }

    col = tolower(static_cast<unsigned char>(position[0])) - 'a';

    row = position[1] - '1';

    if (row < 0 || row >= height || col < 0 || col >= width) {
        return false;
    }

    return true;
}

bool gameClear() {
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            if (!opened[row][col]) {
                return false;
            }
        }
    }

    return true;
}

void waitEnter() {
    cout << "\n엔터키를 누르세요.";

    cin.ignore(numeric_limits<streamsize>::max(),'\n');

    cin.get();
}

void showHint() {
    system("cls");

    // 모든 카드 공개
    printBoard(true);

    cout << "\n3초 후에 다시 가립니다.\n";

    // 3000밀리초 = 3초
    Sleep(3000);

    system("cls");

    // 다시 가린 보드 출력
    printBoard();
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    // 보드 크기 입력
    while (true) {
        cout << "가로와 세로 크기를 입력하세요 (3~6): ";
        cin >> width >> height;

        if (width >= 3 && width <= 6 && height >= 3 && height <= 6) {
            break;
        }

        cout << "3에서 6 사이로 입력하세요.\n";
    }

    makeBoard();
    if ((width * height) % 2 == 1) {
        cout << "칸 수가 홀수라서 조커가 있습니다.\n";
    }
    else {
        cout << "칸 수가 짝수라서 조커가 없습니다.\n";
    }
    printBoard();

    while (true) {
        if (gameClear()) {
            cout << "\n모든 카드를 맞췄습니다!\n";
            cout << "최종 점수: " << score << '\n';
            break;
        }

        if (tryCount >= maxTry) {
            cout << "\n기회를 모두 사용했습니다.\n";
            cout << "최종 점수: " << score << '\n';
            break;
        }

        string first;
        string second;

        cout << "\n두 칸을 입력하세요 (예: a1 c3)\n";
        cout << "r: 재시작, h: 힌트, q: 종료\n";

        cin >> first;

        // 종료
        if (first == "q") {
            break;
        }

        // 재시작
        if (first == "r") {
            makeBoard();

            system("cls");

            cout << "게임을 재시작합니다.\n";
            printBoard();

            continue;
        }

        // 힌트
        if (first == "h") {
            showHint();
            continue;
        }

        // 두 번째 위치 입력
        cin >> second;

        int row1{};
        int col1{};
        int row2{};
        int col2{};

        if (!changePosition(first, row1, col1) || !changePosition(second, row2, col2)) {
            cout << "잘못된 위치입니다.\n";
            continue;
        }

        if (row1 == row2 && col1 == col2) {
            cout << "같은 칸을 두 번 선택할 수 없습니다.\n";
            continue;
        }

        if (opened[row1][col1] || opened[row2][col2]) {
            cout << "이미 열린 카드입니다.\n";
            continue;
        }

        ++tryCount;

        system("cls");

        // 선택한 카드 2개 공개
        printBoard(false, row1, col1, row2,  col2);

        char firstCard = board[row1][col1];
        char secondCard = board[row2][col2];

        // 조커가 선택된 경우
        if (firstCard == '@' || secondCard == '@') {
            char targetCard;

            if (firstCard == '@') {
                targetCard = secondCard;
            }
            else {
                targetCard = firstCard;
            }

            // 선택한 두 카드 열기
            opened[row1][col1] = true;
            opened[row2][col2] = true;

            // 일반 카드의 나머지 짝도 자동으로 열기
            for (int row = 0; row < height; ++row) {
                for (int col = 0; col < width; ++col) {
                    if (board[row][col] == targetCard) {
                        opened[row][col] = true;
                    }
                }
            }

            score += 10;

            cout << "\n조커입니다! 같은 카드가 자동으로 열립니다.\n";

            Sleep(1000);
            system("cls");
            printBoard();
        }
        // 두 문자가 같은 경우
        else if (firstCard == secondCard) {
            opened[row1][col1] = true;
            opened[row2][col2] = true;

            score += 10;

            cout << "\n일치합니다!\n";

            Sleep(1000);
            system("cls");
            printBoard();
        }
        // 두 문자가 다른 경우
        else {
            cout << "\n일치하지 않습니다.\n";

            if (score > 0) {
                --score;
            }

            waitEnter();

            system("cls");
            printBoard();
        }
    }

    setColor(15);

    return 0;
}