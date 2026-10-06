#include <iostream>
#include <vector>
#include <limits>
#include <cstdlib>

#define NOMINMAX
#include <windows.h>

using namespace std;

const int BoardSize = 30;

vector<vector<char>> Board;

char command;
bool firstSelect{ true };

typedef class POS {
public:
    char image{};
    int left{};
    int top{};
    int right{};
    int bottom{};
}Pos;

Pos first{ 'X', 0, 0, 0, 0 };
Pos secound{ '*', 0, 0, 0, 0 };


void makeBoard() {
    Board = vector<vector<char>>(BoardSize, vector<char>(BoardSize, '.'));
    firstSelect = true;
}


void checkBoard(vector<vector<char>>& Board) {
    int boardSize = static_cast<int>(Board.size());

    for (int i = first.top; i <= first.bottom; ++i) {
        for (int j = first.left; j <= first.right; ++j) {

            int firstY = (i % boardSize + boardSize) % boardSize;
            int firstX = (j % boardSize + boardSize) % boardSize;

            Board[firstY][firstX] = first.image;
        }
    }

    for (int i = secound.top; i <= secound.bottom; ++i) {
        for (int j = secound.left; j <= secound.right; ++j) {

            int secoundY = (i % boardSize + boardSize) % boardSize;
            int secoundX = (j % boardSize + boardSize) % boardSize;

            if (Board[secoundY][secoundX] == first.image) {
                Board[secoundY][secoundX] = '#';
            }
            else {
                Board[secoundY][secoundX] = secound.image;
            }
        }
    }
}


void printBoard(vector<vector<char>> Board) {
    checkBoard(Board);

    for (int i = 0; i < Board.size(); ++i) {
        for (int j = 0; j < Board[i].size(); ++j) {
            if (Board[i][j] == '#') {
                // 두 도형이 겹친 부분은 빨간색으로 출력
                SetConsoleTextAttribute(
                    GetStdHandle(STD_OUTPUT_HANDLE),
                    12
                );
            }
            else {
                // 겹치지 않은 부분은 기본 흰색으로 출력
                SetConsoleTextAttribute(
                    GetStdHandle(STD_OUTPUT_HANDLE),
                    15
                );
            }

            cout << Board[i][j] << " ";
        }
        cout << "\n";
    }

    // 보드 출력이 끝난 뒤 콘솔 색상을 원래대로 되돌림
    SetConsoleTextAttribute(
        GetStdHandle(STD_OUTPUT_HANDLE),
        15
    );
}


void inputPos() {
    while (true) {

        cout << "첫 번째 좌표를 입력해주세요 (left top right bottom)\n";

        if (!(cin >> first.left >> first.top >> first.right >> first.bottom)) {
            cout << "정수만 입력해주세요.\n";

            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            continue;
        }

        cout << "두 번째 좌표를 입력해주세요 (left top right bottom)\n";

        if (!(cin >> secound.left >> secound.top >> secound.right >> secound.bottom)) {
            cout << "정수만 입력해주세요.\n";

            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (Board.size() > first.left && first.left >= 0 &&
            Board.size() > first.top && first.top >= 0 &&
            Board.size() > first.right && first.right >= 0 &&
            Board.size() > first.bottom && first.bottom >= 0 &&

            Board.size() > secound.left && secound.left >= 0 &&
            Board.size() > secound.top && secound.top >= 0 &&
            Board.size() > secound.right && secound.right >= 0 &&
            Board.size() > secound.bottom && secound.bottom >= 0 &&

            first.left <= first.right &&
            first.top <= first.bottom &&
            secound.left <= secound.right &&
            secound.top <= secound.bottom) {

            printBoard(Board);

            break;
        }
        else {
            cout << "다시 입력해주세요\n";
        }
    }
}


void moveInBoard(Pos& pos, int boardSize) {

    if (pos.left != 0 && pos.right >= boardSize) {

        int move = pos.right - boardSize + 1;

        pos.left -= move;
        pos.right -= move;
    }

    if (pos.top != 0 && pos.bottom >= boardSize) {

        int move = pos.bottom - boardSize + 1;

        pos.top -= move;
        pos.bottom -= move;
    }
}


void upBoard() {

    int boardSize = Board.size() + 1;

    if (boardSize > BoardSize + 10) {
        cout << "더 이상 보드를 늘릴 수 없습니다.\n";
        return;
    }

    Board = vector<vector<char>>(boardSize, vector<char>(boardSize, '.'));
}


void downBoard() {

    int boardSize = Board.size() - 1;

    if (boardSize < BoardSize - 20) {
        cout << "더 이상 보드를 줄일 수 없습니다.\n";
        return;
    }

    moveInBoard(first, boardSize);
    moveInBoard(secound, boardSize);

    Board = vector<vector<char>>(boardSize, vector<char>(boardSize, '.'));
}


int main()
{
    makeBoard();

    inputPos();

    while (true) {

        cout << "현재 선택된 도형: ";

        if (firstSelect) {
            cout << "첫 번째 도형\n";
        }
        else {
            cout << "두 번째 도형\n";
        }

        cout << "v: 도형 선택 변경\n";
        cout << "명령어를 입력해주세요: ";

        cin >> command;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        system("cls");

        Pos& select = firstSelect ? first : secound;


        // 첫 번째 도형과 두 번째 도형 선택 변경
        if (command == 'v') {
            firstSelect = !firstSelect;

            if (firstSelect) {
                cout << "첫 번째 도형을 선택했습니다.\n";
            }
            else {
                cout << "두 번째 도형을 선택했습니다.\n";
            }
        }


        else if (command == 'x') {
            select.left++;
            select.right++;
        }

        else if (command == 'X') {
            select.left--;
            select.right--;
        }

        else if (command == 'y') {
            select.top++;
            select.bottom++;
        }

        else if (command == 'Y') {
            select.top--;
            select.bottom--;
        }


        else if (command == 's') {

            if (select.right > select.left) {
                select.right--;
            }

            if (select.bottom > select.top) {
                select.bottom--;
            }
        }

        else if (command == 'S') {

            if (select.right - select.left + 1 < static_cast<int>(Board.size())) {
                select.right++;
            }

            if (select.bottom - select.top + 1 < static_cast<int>(Board.size())) {
                select.bottom++;
            }
        }


        else if (command == 'i') {

            if (select.right - select.left + 1 < static_cast<int>(Board.size())) {
                select.right++;
            }
        }

        else if (command == 'I') {

            if (select.right > select.left) {
                select.right--;
            }
        }


        else if (command == 'j') {

            if (select.bottom - select.top + 1 < static_cast<int>(Board.size())) {
                select.bottom++;
            }
        }

        else if (command == 'J') {

            if (select.bottom > select.top) {
                select.bottom--;
            }
        }


        else if (command == 'a') {

            if (select.right - select.left + 1 < static_cast<int>(Board.size())) {
                select.right++;
            }

            if (select.bottom > select.top) {
                select.bottom--;
            }
        }

        else if (command == 'A') {

            if (select.right > select.left) {
                select.right--;
            }

            if (select.bottom - select.top + 1 < static_cast<int>(Board.size())) {
                select.bottom++;
            }
        }


        else if (command == 'b') {

            cout << "첫 번째 도형: "
                << first.right - first.left + 1 << " x "
                << first.bottom - first.top + 1 << " = "
                << (first.right - first.left + 1)
                * (first.bottom - first.top + 1)
                << "\n";

            cout << "두 번째 도형: "
                << secound.right - secound.left + 1 << " x "
                << secound.bottom - secound.top + 1 << " = "
                << (secound.right - secound.left + 1)
                * (secound.bottom - secound.top + 1)
                << "\n";
        }


        else if (command == 'c') {
            upBoard();
        }


        else if (command == 'd') {
            downBoard();
        }


        else if (command == 'r') {

            makeBoard();

            inputPos();

            continue;
        }


        else if (command == 'q') {
            break;
        }


        printBoard(Board);
    }

    return 0;
}
