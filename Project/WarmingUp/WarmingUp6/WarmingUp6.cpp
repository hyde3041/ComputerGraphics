#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <climits>

using namespace std;

// v 한 줄에서 읽은 3차원 정점 좌표를 저장한다.
struct Vertex {
    double x{};
    double y{};
    double z{};
};

// vt 한 줄에서 읽은 2차원 텍스처 좌표를 저장한다.
struct Texture {
    double s{};
    double t{};
};

// f 한 줄은 삼각형 하나를 의미한다.
// vertex에는 정점 인덱스 3개, texture에는 텍스처 인덱스 3개를 저장한다.
struct Face {
    int vertex[3]{};
    int texture[3]{};
    bool hasTexture{false};
};

// 파일에서 읽은 모든 정점, 텍스처, 삼각형 면을 저장하는 목록이다.
vector<Vertex> vertices;
vector<Texture> textures;
vector<Face> faces;

// 두 정점의 x, y, z가 모두 같은지 검사한다.
bool sameVertex(const Vertex& a, const Vertex& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

// 빈 문자열, 문자 포함, 0 이하, int 범위 초과 값은 false를 반환한다.
bool readIndex(const string& text, int& value) {
    if (text.empty()) {
        return false;
    }

    long long number{};

    // 문자열을 한 글자씩 읽어 숫자인지 확인한다.
    for (char ch : text) {
        if (ch < '0' || ch > '9') {
            return false;
        }
        number = number * 10 + (ch - '0');
        if (number > INT_MAX) {
            return false;
        }
    }
    if (number < 1) {
        return false;
    }
    value = static_cast<int>(number);
    return true;
}


bool readFaceValue(const string& text, int& vertex, int& texture, bool& hasTexture) {

    size_t slash = text.find('/');

    // /가 없으면 텍스처 없이 정점 인덱스만 있는 형식
    if (slash == string::npos) {
        hasTexture = false;
        texture = 0;
        return readIndex(text, vertex);
    }

    // /가 두 개 이상이면 오류
    if (text.find('/', slash + 1) != string::npos) {
        return false;
    }

    // / 앞부분과 뒷부분을 각각 정점 번호와 텍스처 번호로 분리
    string vertexText = text.substr(0, slash);
    string textureText = text.substr(slash + 1);

    hasTexture = true;

    return readIndex(vertexText, vertex) &&
           readIndex(textureText, texture);
}

// 입력 파일을 한 줄씩
bool readFile(const string& fileName) {
    ifstream file(fileName);

    if (!file.is_open()) {
        cout << "파일을 열 수 없습니다.\n";
        return false;
    }

    vertices.clear();
    textures.clear();
    faces.clear();

    string line;
    int lineNumber{};

    // getline을 이용해 파일을 한 줄씩
    while (getline(file, line)) {
        ++lineNumber;

        // #부터 줄 끝까지는 주석이므로 파싱하기 전에 제거한다.
        // 예: v 0.5 0.0 0.5 # 1번 정점
        // 처리 후: v 0.5 0.0 0.5
        size_t comment = line.find('#');

        if (comment != string::npos) {
            line = line.substr(0, comment);
        }

        stringstream stream(line);
        string type;

        // 빈 줄이나 #만 있던 줄은 읽을 값이 없으므로 넘어간다.
        if (!(stream >> type)) {
            continue;
        }

        // v x y z 형식의 3차원 정점 처리
        if (type == "v") {
            Vertex vertex;
            string extra;

            // 숫자 3개를 못 읽거나 뒤에 불필요한 값이 남으면 형식 오류
            if (!(stream >> vertex.x >> vertex.y >> vertex.z) || stream >> extra) {
                cout << lineNumber << "번 줄 오류: 정점 형식이 잘못되었습니다.\n";
                return false;
            }

            if (vertex.x < -1.0 || vertex.x > 1.0 ||
                vertex.y < -1.0 || vertex.y > 1.0 ||
                vertex.z < -1.0 || vertex.z > 1.0) {
                cout << lineNumber << "번 줄 오류: 정점 좌표 범위는 -1.0~1.0입니다.\n";
                return false;
            }

            // 이미 저장된 정점과 좌표값이 완전히 같은지 검사
            for (const Vertex& saved : vertices) {
                if (sameVertex(saved, vertex)) {
                    cout << lineNumber << "번 줄 오류: 중복된 정점 좌표입니다.\n";
                    return false;
                }
            }

            // 모든 검사를 통과한 정점을 목록 마지막에 저장
            vertices.push_back(vertex);
        }
        // vt s t 형식의 2차원 텍스처 좌표 처리
        else if (type == "vt") {
            Texture texture;
            string extra;

            if (!(stream >> texture.s >> texture.t) || stream >> extra) {
                cout << lineNumber << "번 줄 오류: 텍스처 형식이 잘못되었습니다.\n";
                return false;
            }

            if (texture.s < 0.0 || texture.s > 1.0 ||
                texture.t < 0.0 || texture.t > 1.0) {
                cout << lineNumber << "번 줄 오류: 텍스처 좌표 범위는 0.0~1.0입니다.\n";
                return false;
            }

            // 모든 검사를 통과한 텍스처 좌표를 저장
            textures.push_back(texture);
        }
        // f v/vt v/vt v/vt 또는 f v v v 형식의 삼각형 면 처리
        else if (type == "f") {
            vector<string> values;
            string value;

            // f 뒤에 남아 있는 인덱스 값들을 모두 문자열로 저장
            while (stream >> value) {
                values.push_back(value);
            }

            // 삼각형은 꼭짓점이 정확히 3개여야 함
            if (values.size() != 3) {
                cout << lineNumber << "번 줄 오류: 삼각형은 꼭짓점이 3개여야 합니다.\n";
                return false;
            }

            Face face;
            bool firstHasTexture{};

            // 인덱스 3개를 각각 정점 번호와 텍스처 번호로 분리
            for (int i = 0; i < 3; ++i) {
                bool hasTexture{};

                if (!readFaceValue(
                        values[i],
                        face.vertex[i],
                        face.texture[i],
                        hasTexture)) {
                    cout << lineNumber << "번 줄 오류: 면 인덱스 형식이 잘못되었습니다.\n";
                    return false;
                }

                // 한 면에서 텍스처 사용 형식이 섞였는지 확인
                if (i == 0) {
                    firstHasTexture = hasTexture;
                }
                else if (firstHasTexture != hasTexture) {
                    cout << lineNumber << "번 줄 오류: 텍스처 인덱스를 동일한 형식으로 입력하세요.\n";
                    return false;
                }
            }

            face.hasTexture = firstHasTexture;

            // 같은 정점 번호를 두 번 이상 사용 했는가?
            if (face.vertex[0] == face.vertex[1] ||
                face.vertex[0] == face.vertex[2] ||
                face.vertex[1] == face.vertex[2]) {
                cout << lineNumber << "번 줄 오류: 삼각형의 정점 인덱스가 중복됩니다.\n";
                return false;
            }

            // f가 실제로 존재하는 정점과 텍스처 번호를 가리키는지 검사
            for (int i = 0; i < 3; ++i) {
                if (face.vertex[i] > static_cast<int>(vertices.size())) {
                    cout << lineNumber << "번 줄 오류: 정점 인덱스가 범위를 벗어났습니다.\n";
                    return false;
                }

                if (face.hasTexture &&
                    face.texture[i] > static_cast<int>(textures.size())) {
                    cout << lineNumber << "번 줄 오류: 텍스처 인덱스가 범위를 벗어났습니다.\n";
                    return false;
                }
            }

            // 모든 검사를 통과한 삼각형 면을 목록에 저장
            faces.push_back(face);
        }
        else {
            // v, vt, f 이외의 단어로 시작하는 줄은 오류
            cout << lineNumber << "번 줄 오류: 허용되지 않는 문자값입니다.\n";
            return false;
        }
    }

    return true;
}

// 파싱한 삼각형의 실제 정점 좌표와 텍스처 좌표를 결과 파일에 쓴다.
bool writeFile(const string& fileName) {
    ofstream file(fileName);

    if (!file.is_open()) {
        cout << "결과 파일을 만들 수 없습니다.\n";
        return false;
    }

    // 모든 실수를 소수점 아래 6자리까지 출력한다.
    file << fixed << setprecision(6);

    // 저장된 f의 개수만큼 삼각형 정보를 출력한다.
    for (int i = 0; i < static_cast<int>(faces.size()); ++i) {
        const Face& face = faces[i];

        file << "Face " << i + 1 << " ("
             << face.vertex[0] << ", "
             << face.vertex[1] << ", "
             << face.vertex[2] << "): vertex ";

        // 파일 인덱스는 1부터, vector 인덱스는 0부터 시작하므로 1을 뺀다.
        for (int j = 0; j < 3; ++j) {
            const Vertex& vertex = vertices[face.vertex[j] - 1];

            file << "(" << vertex.x << ", "
                 << vertex.y << ", "
                 << vertex.z << ") ";
        }

        file << '\n';

        // 해당 f에 텍스처 인덱스가 있을 때만 텍스처 좌표를 출력한다.
        if (face.hasTexture) {
            file << "texture ";

            for (int j = 0; j < 3; ++j) {
                const Texture& texture = textures[face.texture[j] - 1];

                file << "(" << texture.s << ", "
                     << texture.t << ") ";
            }

            file << '\n';
        }
    }

    file << "No duplicate vertex value\n";
    return true;
}

int main() {
    string inputFile;
    string outputFile;

    cout << "입력 파일 이름: ";
    getline(cin, inputFile);

    cout << "출력 파일 이름: ";
    getline(cin, outputFile);

    // 입력 파일을 읽다가 오류가 발견되면 프로그램을 종료한다.
    if (!readFile(inputFile)) {
        return 1;
    }

    // 유효한 f 데이터가 하나도 없으면 출력할 삼각형이 없다.
    if (faces.empty()) {
        cout << "오류: 삼각형 면 데이터가 없습니다.\n";
        return 1;
    }

    // 검사에 통과한 삼각형 정보를 결과 파일에 저장한다.
    if (!writeFile(outputFile)) {
        return 1;
    }

    cout << "정점 " << vertices.size() << "개\n";
    cout << "텍스처 " << textures.size() << "개\n";
    cout << "삼각형 " << faces.size() << "개\n";
    cout << "결과를 " << outputFile << " 파일에 저장했습니다.\n";

    return 0;
}
