from pathlib import Path
import json, re

ROOT = Path(__file__).resolve().parent
RESULTS = {r['key']:r for r in json.loads((ROOT/'results.json').read_text(encoding='utf-8'))}
CHAPTERS = [
 {'id':'02','title':'운영체제 개요','source':[1,17],'summary':[1],'questions':[],'count':0},
 {'id':'04','title':'프로세스','source':[18,30],'summary':[],'questions':[28,29],'count':8},
 {'id':'05','title':'프로세스 API','source':[31,41],'summary':[],'questions':[40,41],'count':8},
 {'id':'06','title':'제한적 직접 실행','source':[42,57],'summary':[4],'questions':[56,57],'count':2},
 {'id':'07','title':'CPU 스케줄링 기본','source':[58,71],'summary':[5],'questions':[71],'count':7},
 {'id':'08','title':'멀티 레벨 피드백 큐','source':[72,83],'summary':[6],'questions':[83],'count':6},
 {'id':'09','title':'비례 배분과 CFS','source':[84,97],'summary':[7],'questions':[92],'count':5},
 {'id':'10','title':'멀티프로세서 스케줄링','source':[98,111],'summary':[8],'questions':[110,111],'count':9},
 {'id':'13','title':'주소 공간','source':[112,122],'summary':[],'questions':[122],'count':8},
 {'id':'14','title':'메모리 API','source':[123,133],'summary':[],'questions':[132,133],'count':9},
 {'id':'15','title':'주소 변환의 원리','source':[134,147],'summary':[],'questions':[147],'count':5},
 {'id':'16','title':'세그멘테이션','source':[148,159],'summary':[],'questions':[159],'count':5},
 {'id':'17','title':'빈 공간 관리','source':[160,176],'summary':[11],'questions':[176],'count':7},
 {'id':'18','title':'페이징','source':[177,190],'summary':[12],'questions':[189,190],'count':4},
 {'id':'19','title':'TLB','source':[191,208],'summary':[13],'questions':[206,207,208],'count':7},
 {'id':'20','title':'더 작은 페이지 테이블','source':[209,223],'summary':[14],'questions':[223],'count':3},
 {'id':'21','title':'스왑 메커니즘','source':[224,235],'summary':[15],'questions':[234,235],'count':7},
 {'id':'22','title':'페이지 교체 정책','source':[236,257],'summary':[16],'questions':[253],'count':5},
 {'id':'23','title':'VAX/VMS·Linux 가상 메모리','source':[258,280],'summary':[17,18],'questions':[],'count':0},
]
ANSWERS={c['id']:[] for c in CHAPTERS}

def T(headers, rows, widths=None): return {'type':'table','headers':headers,'rows':rows,'widths':widths}
def C(code, caption='예시 코드'): return {'type':'code','caption':caption,'text':code.strip()}
def F(name, caption): return {'type':'figure','name':name,'caption':caption}
def q(ch,n,title,answer,explanation,*extra):
    ANSWERS[ch].append({'n':n,'title':title,'answer':answer,'explanation':explanation,'extra':list(extra)})
def runs(key): return RESULTS[key]['stdout']
def cpu_stats(key):
    t=runs(key)
    return [re.search(r'Total Time (\d+)',t).group(1),re.search(r'CPU Busy\s+(\d+) \(([^)]+)\)',t).group(2)]
def finished(key): return re.search(r'Finished time (\d+)',runs(key)).group(1)
def done(key): return [(int(a),int(b)) for a,b in re.findall(r'JOB (\d+) DONE at time (\d+)',runs(key))]

q('04',1,'CPU만 사용하는 두 프로세스',
  '총 10틱, CPU 이용률 <b>100%</b>다.',
  '각 프로세스가 CPU 명령 5개를 실행한다. I/O로 막히는 순간이 없어 CPU가 10틱 내내 일한다. 공식 프로그램의 -l 5:100,5:100 -c -p 결과로 확인했다.')
q('04',2,'CPU 작업을 먼저 실행한 경우',
  '기본 I/O 길이 5에서 총 <b>11틱</b>, CPU 이용률 <b>54.55%</b>다.',
  'CPU 명령 4틱 뒤 I/O 요청 1틱, 대기 5틱, I/O 완료 명령(io_done) 1틱이 이어진다. 이 버전의 시뮬레이터는 요청과 완료 처리도 CPU 시간으로 세므로 CPU 사용은 6틱이다. 시스템의 실제 문맥 교환 시간 측정값을 뜻하는 것은 아니다.')
q('04',3,'I/O 작업을 먼저 실행한 경우',
  '총 <b>7틱</b>, CPU 이용률 <b>85.71%</b>다.',
  '1틱에 I/O를 요청하고, 대기하는 동안 다른 프로세스의 CPU 명령 4개를 처리한다. 6틱에는 CPU가 쉬고 7틱에 I/O 완료 처리를 한다. 계산과 I/O가 겹쳐 2번의 11틱보다 짧다.')
q('04',4,'SWITCH_ON_END',
  '총 <b>11틱</b>, CPU 이용률 <b>54.55%</b>다.',
  'I/O 프로세스가 끝날 때까지 전환하지 않으므로 I/O 대기 5틱 동안 CPU가 논다. 원문의 4:200은 퍼센트 표기로는 오기다. 이 프로그램은 200을 항상 CPU 명령을 고르는 값으로 처리하며 4:100으로 고쳐도 결과가 같다.')
q('04',5,'SWITCH_ON_IO',
  '총 <b>7틱</b>, CPU 이용률 <b>85.71%</b>다.',
  'I/O가 시작되면 다른 준비 프로세스로 전환한다. I/O 프로세스를 앞에 둔 3번과 같은 상황이므로 대기와 계산을 겹칠 수 있다.')
q('04',6,'I/O 완료 후 나중에 재실행',
  'IO_RUN_LATER: 총 <b>31틱</b>, CPU 이용률 <b>67.74%</b>다.',
  'I/O가 끝났어도 실행 중인 CPU 작업을 계속한다. 다음 I/O 요청이 늦어지고, CPU 작업들이 끝난 뒤 나머지 I/O만 기다리는 구간이 생긴다. CPU는 총 21틱, I/O 장치는 15틱 사용된다.')
q('04',7,'I/O 완료 후 즉시 재실행',
  'IO_RUN_IMMEDIATE: 총 <b>21틱</b>, CPU 이용률 <b>100%</b>다.',
  'I/O 작업을 바로 실행해 다음 I/O를 빨리 요청하면, 그 대기 동안 CPU 작업을 처리할 수 있다. 이 워크로드에서는 I/O와 계산이 잘 겹쳐 CPU 유휴 구간을 없앤다. 모든 워크로드에서 항상 이 정책이 최적인 것은 아니다.')
rows=[]
for s in [1,2,3]:
    for sw in ['SWITCH_ON_IO','SWITCH_ON_END']:
        for io in ['IO_RUN_LATER','IO_RUN_IMMEDIATE']:
            rows.append([s,sw.replace('SWITCH_ON_',''),io.replace('IO_RUN_',''),*cpu_stats(f'04_8_s{s}_{sw}_{io}')])
q('04',8,'시드와 I/O·전환 정책 비교',
  '아래 표가 -l 3:50,3:50, 기본 I/O 길이 5의 검산 결과다.',
  '같은 시드라도 두 정책이 일정을 달리 만든다. SWITCH_ON_IO는 다른 작업으로 대기 시간을 덮는다. IMMEDIATE와 LATER가 같은 결과인 시드도 있으며, 시드 3에서는 즉시 재실행이 1틱을 절약한다. 확률 50%는 명령 생성 규칙이므로 실제 짧은 명령열의 CPU 비율이 정확히 50%일 필요는 없다.',
  T(['시드','전환','완료 후','총 틱','CPU 이용률'],rows,[.10,.15,.25,.15,.35]))

q('05',1,'fork 전후 변수 x',
  '자식은 처음에 x=100을 본다. 이후 부모와 자식이 변경한 x는 서로 독립이다.',
  'fork가 주소 공간의 논리적 복사본을 만든다. COW로 물리 페이지를 잠시 공유해도 쓰기 때 분리된다. 아래에서 자식은 200, 부모는 300을 출력한다.',
  C('''#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main(void) {
    int x = 100;
    pid_t p = fork();
    if (p < 0) { perror("fork"); return 1; }
    if (p == 0) {
        printf("child initially: %d\\n", x);
        x = 200;
        printf("child changed: %d\\n", x);
        return 0;
    }
    x = 300;
    waitpid(p, NULL, 0);
    printf("parent changed: %d\\n", x);
    return 0;
}'''))
q('05',2,'fork 전 open한 파일 디스크립터',
  '부모와 자식 모두 파일을 쓸 수 있다. 같은 열린 파일 설명(open file description)의 오프셋과 상태 플래그를 공유한다.',
  'FD 번호는 각각의 프로세스 테이블에 있지만 동일한 커널 객체를 가리킨다. 각 write의 실행 순서는 정해지지 않는다. 레코드 순서를 보장하려면 동기화해야 한다. 표준 입출력의 사용자 공간 버퍼와 커널의 파일 오프셋을 혼동하지 않는다.',
  C('''#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
int main(void) {
    int fd = open("fork-file.txt", O_WRONLY|O_CREAT|O_TRUNC, 0600);
    if (fd < 0) { perror("open"); return 1; }
    pid_t p = fork();
    if (p < 0) { perror("fork"); close(fd); return 1; }
    const char *s = p == 0 ? "child\\n" : "parent\\n";
    size_t n = p == 0 ? 6 : 7;
    if (write(fd, s, n) != (ssize_t)n) perror("write");
    close(fd);
    if (p != 0) waitpid(p, NULL, 0);
    return 0;
}'''))
q('05',3,'wait 없이 hello를 먼저 출력',
  '가능하다. 파이프 같은 명시적인 동기화 수단을 쓰면 된다. sleep만으로는 순서를 보장하지 못한다.',
  '자식이 hello를 출력하고 fflush한 뒤 파이프에 신호를 쓴다. 부모는 신호를 읽을 때까지 막혀 있으므로 goodbye가 나중에 출력된다. 이 예는 wait/waitpid를 호출하지 않고 출력 순서만 동기화한다.',
  C('''#include <stdio.h>
#include <unistd.h>
int main(void) {
    int f[2]; char signal;
    if (pipe(f) < 0) { perror("pipe"); return 1; }
    pid_t p = fork();
    if (p < 0) { perror("fork"); return 1; }
    if (p == 0) {
        close(f[0]);
        puts("hello"); fflush(stdout);
        if (write(f[1], "x", 1) != 1) _exit(1);
        close(f[1]); _exit(0);
    }
    close(f[1]);
    if (read(f[0], &signal, 1) == 1) puts("goodbye");
    close(f[0]);
    return 0;
}'''))
q('05',4,'exec 계열 여섯 가지 변형',
  'l은 가변 인자 목록, v는 argv 배열, p는 PATH 검색, e는 명시적 환경 배열을 뜻한다.',
  '호출자가 가진 인자의 형태와 실행 파일 검색·환경 설정 요구에 맞춘 인터페이스다. 성공하면 자식의 프로그램 이미지가 ls로 바뀌고 이전 코드로 돌아오지 않는다. 실행 파일 이름만 바뀌며 PID는 유지된다.',
  T(['함수','인자','실행 파일','환경'],[
   ['execl','목록','경로','상속'],['execle','목록','경로','명시'],['execlp','목록','PATH','상속'],
   ['execv','배열','경로','상속'],['execvp','배열','PATH','상속'],['execve','배열','경로','명시']]),
  C('''#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main(int argc, char **argv) {
    if (argc != 2) return 1; /* choose 1..6 */
    pid_t p = fork();
    if (p < 0) { perror("fork"); return 1; }
    if (p == 0) {
        char *a[] = {"ls", "-l", NULL};
        char *e[] = {"PATH=/usr/bin:/bin", NULL};
        switch (atoi(argv[1])) {
        case 1: execl("/bin/ls","ls","-l",(char*)NULL); break;
        case 2: execle("/bin/ls","ls","-l",(char*)NULL,e); break;
        case 3: execlp("ls","ls","-l",(char*)NULL); break;
        case 4: execv("/bin/ls",a); break;
        case 5: execvp("ls",a); break;
        case 6: execve("/bin/ls",a,e); break;
        default: _exit(2);
        }
        perror("exec"); _exit(1);
    }
    waitpid(p, NULL, 0);
    return 0;
}'''))
q('05',5,'wait의 반환값과 자식의 wait',
  '성공한 wait는 회수한 자식의 PID를 반환한다. 종료 코드는 status에서 매크로로 해석한다. 자식에게 자식이 없다면 -1과 ECHILD를 얻는다.',
  'PID와 종료 상태는 다른 값이다. 아래의 자식은 wait에 실패한 뒤 종료 코드 7로 끝나며 부모는 그 PID와 종료 코드 7을 확인한다. 일반적으로 wait는 여러 자식 중 먼저 회수 가능한 하나를 기다린다.',
  C('''#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
int main(void) {
    pid_t p = fork();
    if (p < 0) { perror("fork"); return 1; }
    if (p == 0) {
        pid_t r = wait(NULL);
        fprintf(stderr,"child wait=%ld ECHILD=%d\\n",
                (long)r, errno == ECHILD);
        _exit(7);
    }
    int status; pid_t r;
    do { r = wait(&status); } while (r < 0 && errno == EINTR);
    if (r < 0) { perror("wait"); return 1; }
    printf("reaped=%ld exit=%d\\n",(long)r,
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    return 0;
}'''))
q('05',6,'waitpid를 쓰는 경우',
  '특정 자식 PID를 기다리거나 WNOHANG으로 비차단 확인할 때 유용하다.',
  'waitpid(pid,&status,0)은 지정한 자식, waitpid(-1,&status,0)은 임의의 자식이다. WNOHANG에서 아직 회수할 상태가 없으면 0, 회수하면 PID, 오류면 -1이다. EINTR이면 재시도한다. 여러 자식을 가진 셸·서버에서 대상 선택이 필요하다.',
  C('''/* replace the parent wait loop in question 5 */
int status;
pid_t r = waitpid(p, &status, WNOHANG);
if (r == 0) {
    /* child still running: other work could be done here */
    do { r = waitpid(p, &status, 0); }
    while (r < 0 && errno == EINTR);
}
if (r < 0) perror("waitpid");
else if (WIFEXITED(status))
    printf("exit=%d\\n", WEXITSTATUS(status));''','5번 프로그램 부모 부분의 교체 코드'))
q('05',7,'stdout을 닫은 자식의 printf',
  '표준 출력으로는 정상 출력되지 않는다. 실제 쓰기 시 닫힌 FD 때문에 EBADF가 발생할 수 있다.',
  'printf가 사용자 공간 버퍼에만 적었다면 바로 성공처럼 보일 수도 있다. fflush나 종료 때 버퍼를 내보내면서 실패가 드러난다. 따라서 printf의 결과뿐 아니라 fflush·ferror도 확인해야 한다. 오류 보고에는 열려 있는 stderr를 쓴다.',
  C('''#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
int main(void) {
    pid_t p = fork();
    if (p < 0) return 1;
    if (p == 0) {
        close(STDOUT_FILENO);
        errno = 0;
        int r = printf("hello\\n");
        int f = fflush(stdout);
        fprintf(stderr,"printf=%d flush=%d error=%d errno=%d\\n",
                r,f,ferror(stdout),errno);
        _exit(0);
    }
    waitpid(p,NULL,0);
    return 0;
}'''))
q('05',8,'두 자식을 pipe로 연결',
  '생산자의 stdout을 파이프 쓰기 끝에, 소비자의 stdin을 읽기 끝에 dup2로 연결한다.',
  '아래는 echo hello | wc -c에 해당하며 결과는 줄바꿈을 포함한 6이다. 두 자식과 부모 모두 불필요한 파이프 끝을 닫아야 한다. 부모가 쓰기 끝을 열어 두면 소비자가 EOF를 받지 못해 기다릴 수 있다.',
  C('''#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
int main(void) {
    int f[2];
    if (pipe(f) < 0) return 1;
    pid_t a = fork();
    if (a < 0) return 1;
    if (a == 0) {
        if (dup2(f[1],STDOUT_FILENO) < 0) _exit(1);
        close(f[0]); close(f[1]);
        execlp("echo","echo","hello",(char*)NULL);
        perror("echo"); _exit(1);
    }
    pid_t b = fork();
    if (b < 0) return 1;
    if (b == 0) {
        if (dup2(f[0],STDIN_FILENO) < 0) _exit(1);
        close(f[0]); close(f[1]);
        execlp("wc","wc","-c",(char*)NULL);
        perror("wc"); _exit(1);
    }
    close(f[0]); close(f[1]);
    waitpid(a,NULL,0); waitpid(b,NULL,0);
    return 0;
}'''))

q('06',1,'측정 과제 1: 시스템 콜 비용과 타이머',
  '고정된 정답 시간은 없다. 평균 비용은 (반복 호출의 총 시간 - 루프 기준 비용)/호출 횟수로 추정한다.',
  'gettimeofday의 출력 단위가 μs라고 해서 정확도·분해능도 1 μs인 것은 아니다. 연속 읽기의 차이, 측정 오버헤드, 전체 측정 시간이 충분한지 확인한다. 아래는 Linux에서 read 시스템 콜을 직접 반복한다. 실측을 수행한 수치가 아닌 재현 코드다. CLOCK_MONOTONIC은 벽시계 보정에 덜 영향을 받는다.',
  C('''#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
static double ns(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC,&t);
    return t.tv_sec*1e9+t.tv_nsec;
}
int main(int argc,char **argv) {
    long n = argc > 1 ? atol(argv[1]) : 1000000;
    if (n <= 0) return 1;
    int fd = open("/dev/null",O_RDONLY);
    if (fd < 0) return 1;
    char b; volatile long sink = 0;
    double t = ns();
    for (long i=0;i<n;i++) sink += 0;
    double baseline = ns()-t;
    t=ns();
    for (long i=0;i<n;i++) {
        if (syscall(SYS_read,fd,&b,0) < 0) return 1;
        sink += 0;
    }
    double elapsed=ns()-t;
    printf("raw_ns=%.2f estimated_ns=%.2f\\n",
           elapsed/n,(elapsed-baseline)/n);
    close(fd); return 0;
}''','syscall-cost.c: gcc -O2 syscall-cost.c -o syscall-cost'))
q('06',2,'측정 과제 2: 문맥 교환 비용',
  '두 프로세스를 같은 CPU에 고정하고 파이프 왕복을 반복한다. 왕복 평균을 2로 나누면 단방향 전환과 IPC를 포함한 비용 추정치다.',
  '한 왕복에는 보통 부모→자식→부모 전환이 있다. read/write와 파이프 전달 비용도 들어가므로 순수한 레지스터 교환 비용이라고 단정하지 않는다. 여러 CPU에 흩어지면 두 프로세스가 동시에 실행돼 같은 CPU의 문맥 교환을 측정하지 못한다. 워밍업 후 여러 번 측정해 중앙값과 변동폭을 보고한다.',
  C('''#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
static double ns(void) {
    struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t);
    return t.tv_sec*1e9+t.tv_nsec;
}
int main(int argc,char **argv) {
    int n=argc>1 ? atoi(argv[1]) : 100000;
    if(n<=0) return 1;
    cpu_set_t set; CPU_ZERO(&set);
    int cpu=sched_getcpu(); if(cpu<0) return 1;
    CPU_SET(cpu,&set);
    if(sched_setaffinity(0,sizeof(set),&set)<0) return 1;
    int a[2],b[2]; char v='x';
    if(pipe(a)<0 || pipe(b)<0) return 1;
    pid_t p=fork(); if(p<0) return 1;
    if(p==0) {
        close(a[1]); close(b[0]);
        while(read(a[0],&v,1)==1)
            if(write(b[1],&v,1)!=1) _exit(1);
        _exit(0);
    }
    close(a[0]); close(b[1]);
    double t=0;
    for(int i=0;i<n+1000;i++) {
        if(i==1000) t=ns();
        if(write(a[1],&v,1)!=1 || read(b[0],&v,1)!=1) return 1;
    }
    double elapsed=ns()-t;
    printf("round_trip_ns=%.2f one_way_including_IPC_ns=%.2f\\n",
           elapsed/n,elapsed/(2*n));
    close(a[1]); close(b[0]); waitpid(p,NULL,0);
    return 0;
}''','switch-cost.c: gcc -O2 switch-cost.c -o switch-cost'))

q('07',1,'길이 200인 작업 3개: FIFO와 SJF',
  '둘 다 응답 시간 0·200·400, 반환 시간 200·400·600이다. 평균은 각각 <b>200</b>, <b>400</b>이다.',
  '모두 시각 0에 도착하고 작업 길이가 같으면 어떤 동률 순서로 실행해도 평균이 같다. 교환 비용 0, 비선점 실행을 가정한다.')
q('07',2,'길이 100·200·300의 FIFO와 SJF',
  'FIFO의 입력 순서가 100→200→300이면 SJF와 같다. 응답 0·100·300, 반환 100·300·600, 평균 응답 <b>133.33</b>, 평균 반환 <b>333.33</b>이다.',
  'SJF는 오름차순으로 실행한다. FIFO 순서가 300→200→100이면 응답 0·300·500, 반환 300·500·600으로 평균 응답 266.67, 평균 반환 466.67이 된다. 입력 순서를 밝히지 않으면 FIFO 수치가 하나로 정해지지 않는다.')
q('07',3,'RR, 퀀텀 1',
  'A·B·C 순서로 첫 실행은 0·1·2, 완료는 298·499·600이다. 평균 응답 <b>1</b>, 평균 반환 <b>465.67</b>이다.',
  '첫 99라운드가 297틱을 사용한다. 다음 A 실행으로 298에 A가 끝난다. B·C만 남아 번갈아 실행하다 B가 499에 끝나고 C가 600에 끝난다. 명령 실행 구간과 완료 시각을 혼동하지 않는다.',
  T(['작업','길이','응답','반환'],[['A',100,0,298],['B',200,1,499],['C',300,2,600]]))
q('07',4,'FIFO와 SJF의 반환 시간이 같은 워크로드',
  '동시 도착·비선점 조건에서 FIFO 순서가 길이의 오름차순이면 같다. 길이가 모두 같거나 작업 하나뿐이어도 같다.',
  '두 알고리즘이 동일한 실행 순서를 만들기 때문이다. 특정 평균만 우연히 같은 경우와 모든 작업의 일정이 같은 조건을 구분한다.')
q('07',5,'SJF와 RR의 응답 시간이 같은 조건',
  '동시 도착·같은 정렬 순서에서 RR의 각 첫 퀀텀이 해당 작업을 끝낼 만큼 크면 SJF와 같다. 충분조건은 q≥최대 작업 길이와 SJF 순서의 준비 큐다.',
  '마지막 작업의 길이는 뒤 작업의 초기 응답에 영향이 없으므로, 더 약한 충분조건은 마지막을 제외한 모든 작업이 첫 퀀텀에 끝나는 것이다. 작업 하나만 있는 경우도 항상 같다. q가 충분히 커도 큐 순서가 다르면 SJF와 같지 않을 수 있다.')
q('07',6,'작업 길이가 늘면 SJF 응답 시간은?',
  '정렬 순서가 유지되고 앞선 작업들의 길이가 늘면 뒤 작업의 응답 시간이 늘어난다. 모든 길이를 k배 하면 응답·반환 시간도 k배다.',
  '정렬된 길이 L1…Ln에서 i번째 응답은 앞의 i-1개 길이의 합이다. 마지막 작업만 길어지면 그 작업의 최초 응답과 다른 작업의 최초 응답은 변하지 않는다. scheduler.py -l 100,200,300 -p SJF -c와 -l 200,400,600을 비교하면 된다.')
q('07',7,'RR 퀀텀과 최악의 초기 응답',
  '동시에 준비된 N개 작업, 교환 비용 0이면 마지막 작업의 초기 응답은 최대 <b>(N-1)q</b>다.',
  '앞의 N-1개 작업이 각각 q씩 사용하면 그만큼 기다린다. 어떤 작업이 일찍 끝나면 더 짧다. 전환당 비용 c를 앞 작업 사이와 목표 작업 진입까지 세면 (N-1)(q+c)로 모델링할 수 있다. 새 도착·다른 우선순위·초기 디스패치 비용이 있으면 그 조건을 추가한다.')

mlfqrows=[]
for s in [0,1,2]:
    t=runs(f'08_1_s{s}')
    lens=re.findall(r'Job\s+\d+: startTime\s+\d+ - runTime\s+(\d+)',t)
    rr=[(int(a),b,c) for a,b,c in re.findall(r'\[ time (\d+) \] Run JOB (\d+) at PRIORITY (\d+)',t)]
    gs=[]
    for a,b,c in rr:
        if gs and gs[-1][2:]==[b,c] and gs[-1][1]==a: gs[-1][1]=a+1
        else: gs.append([a,a+1,b,c])
    mlfqrows.append([s,','.join(lens),' / '.join(f'{a}-{b}: J{j}(Q{lev})' for a,b,j,lev in gs)])
q('08',1,'2개 큐·2개 작업의 무작위 추적',
  '예시 조건 -n 2 -j 2 -m 20 -M 0 -q 5, boost 없음, 단계별 할당량 1이다. 검산 추적은 아래와 같다.',
  'Q1이 높은 큐다. 처음 각각 높은 큐에서 최대 5틱씩 실행한 뒤 남으면 Q0으로 강등한다. 표의 연속 구간은 같은 작업의 인접 실행을 합친 것이며, 낮은 큐의 퀀텀 경계가 없어졌다는 뜻은 아니다.',
  T(['시드','J0,J1 길이','실행 구간 [시작,끝)'],mlfqrows,[.12,.22,.66]))
q('08',2,'본문 예제 재현',
  '아래 명령의 -l은 시작 시각, CPU 길이, I/O 빈도를 지정한다. 본문 그림의 특성별 재현 예를 제시했다.',
  '-Q의 값은 높은 큐부터 적는다. 옛 I/O 우선순위 유지 규칙은 -S로, 새 누적 회계는 -S 없이 실행한다. 입출력 시간·boost 주기·큐별 퀀텀을 그림과 일치시켜야 동일한 일정이 된다.',
  T(['예제','mlfq.py 뒤의 인자'],[
   ['CPU 작업 하나','-l 0,200,0 -Q 10,10,10 -c'],
   ['나중에 도착한 짧은 작업','-l 0,200,0:100,20,0 -Q 10,10,10 -c'],
   ['짧은 CPU 버스트와 I/O','-l 0,200,0:100,20,1 -Q 10,10,10 -S -c'],
   ['주기적 boost','-l 0,200,0:100,50,1:100,50,1 -Q 10,10,10 -S -B 50 -c'],
   ['새 시간 회계','-l 0,200,0:100,50,9 -Q 10,10,10 -i 1 -c'],
   ['큐별 퀀텀 차이','-l 0,200,0:0,200,0 -Q 10,20,40 -c'],
  ],[.29,.71]))
q('08',3,'MLFQ를 RR처럼 만들기',
  '큐를 하나만 두고 모든 작업에 같은 퀀텀을 적용한다: -n 1 -q 2.',
  '모든 작업이 같은 우선순위이므로 강등이 실행 대상의 우선순위를 달리 만들지 않는다. -l 0,10,0:0,10,0의 응답은 0·2, 반환은 18·20으로 보통 RR과 같다. I/O 완료 앞삽입(-I)은 끄면 기본 FIFO 준비 큐에 더 가깝다.')
q('08',4,'옛 규칙을 이용한 CPU 99% 점유',
  '예: -l 0,1000,0:0,1000,99 -n 2 -q 100 -i 1 -S -I -c.',
  'CPU 작업 J0은 처음 100틱을 쓰고 낮은 큐로 내려간다. J1은 99틱마다 1틱의 I/O를 하며, -S가 사용 시간을 초기화해 높은 큐에 남는다. 워밍업 뒤 J1의 반복 구간에서 100틱당 약 99틱을 점유한다. 시작·종료를 포함한 전체 생애 몫을 99%라고 주장하는 것은 아니다. -S를 끄면 누적 사용량이 100틱을 넘을 때 강등되어 조작을 막는다.')
q('08',5,'10 ms 퀀텀으로 최소 5% 보장',
  '매 boost 주기마다 해당 작업이 높은 큐의 10 ms를 온전히 보장받는다는 가정이면 <b>B≤200 ms</b>다.',
  '10/B≥0.05이므로 B≤200이다. 다만 작업 수·큐 순서·도착 패턴이 명시되지 않으면 -B 200만으로 무조건 5%를 보장할 수 없다. 매번 10 ms를 받는다는 조건 자체가 필요하다. 예컨대 동시에 21개 작업 모두에게 최소 5%를 보장하는 것은 한 CPU에서 불가능하다.')
q('08',6,'I/O 완료 작업의 앞삽입·뒤삽입',
  '-I를 켜면 I/O가 끝난 작업을 현재 큐의 앞에 넣는다. 예시에서 J1의 반환 시간은 <b>40→22</b>로 줄고 J0은 50으로 같다.',
  '검산 조건은 -l 0,40,0:0,10,2 -n 1 -q 5 -i 1이다. 앞삽입은 다시 짧은 CPU 버스트를 실행할 기회를 빨리 줘 I/O를 겹치기 좋다. 같은 우선순위의 CPU 작업은 더 오래 기다릴 수 있어 공정성과 응답성 사이의 절충이 있다.')

lotteryrows=[]
for s in [1,2,3]:
    t=runs(f'09_1_s{s}')
    jobs=re.findall(r'Job (\d+) \( length = (\d+), tickets = (\d+) \)',t)
    seq=re.findall(r' -> Run (\d+)',t)
    lotteryrows.append([s,' / '.join(f'J{j}: {l}/{tk}' for j,l,tk in jobs),' '.join(seq),', '.join(f'J{j}={tm}' for j,tm in done(f'09_1_s{s}'))])
q('09',1,'시드 1·2·3의 추첨 일정',
  '기본값: 작업 3개, 최대 길이 10, 최대 티켓 100, 퀀텀 1이다. 전체 실행 순서와 완료 시각은 아래와 같다.',
  '티켓의 누적 구간에 당첨 번호가 들어간 작업이 실행된다. 끝난 작업의 티켓은 제거하므로 다음 추첨의 전체 티켓 수가 줄어든다. 숫자는 작업 번호이며, 1개 숫자당 1틱 실행이다.',
  T(['시드','작업: 길이/티켓','실행 순서','완료 시각'],lotteryrows,[.08,.28,.40,.24]))
q('09',2,'티켓 1 대 100의 불균형',
  '두 작업이 함께 준비되어 있을 때 J0의 선택 확률은 1/101≈0.99%다. J1이 끝나기 전에 J0이 실행될 수도 있다.',
  'J1이 10번 연속 뽑혀 먼저 끝나는 확률은 (100/101)^10이다. 그 전에 J0이 적어도 한 번 실행될 확률은 약 9.47%다. 시드 1·3에서는 J1이 10틱에, 시드 2에서는 11틱에 끝나므로 시드 2에서 J0도 미리 한 번 실행됐다. 최종 J0 완료는 모두 20틱이다. 낮은 확률은 실행 금지가 아니며 짧은 구간의 공정성은 보장되지 않는다.')
fair=[]
for s in [1,2,3]:
    ts=[t for _,t in done(f'09_3_s{s}')]
    fair.append([s,*[dict(done(f'09_3_s{s}'))[j] for j in [0,1]],f'{min(ts)/max(ts):.3f}'])
q('09',3,'같은 길이·같은 티켓의 불공정성',
  '완료 시간 비율 U=빠른 완료/느린 완료로 정의하면, U가 1에 가까울수록 공정하다. 검산 수치는 아래와 같다.',
  '길이가 각각 100이므로 총 완료 시간은 200틱이다. 동일 티켓이어도 추첨 결과가 흔들려 한 작업이 먼저 끝난다. 몇 개의 시드 결과를 전체 확률 분포로 단정하지 말고 여러 시드의 평균·분산을 확인한다.',
  T(['시드','J0 완료','J1 완료','U'],fair))
q('09',4,'퀀텀이 커질 때',
  '보통 단기 불공정성이 커진다. q=10의 시드 1·2·3에서 U는 각각 0.800·0.950·0.950이다.',
  '같은 총 CPU 시간에서 추첨 횟수가 줄어 평균화 효과가 약해지고, 한 번 당첨된 작업이 큰 CPU 몫을 받는다. 다만 모든 개별 시드에서 반드시 더 나빠지는 것은 아니다. q가 작업 길이보다 커지면 먼저 당첨된 작업이 통째로 끝나 RR과 같은 교대 효과도 사라진다.')
q('09',5,'그래프와 Stride 비교',
  '아래는 동일 티켓 작업 2개, q=1에서 길이별 500개 시드의 평균 U다. Stride는 결정적 교대로 더 빠르게 1에 가까워진다.',
  '추첨 모형은 공식 코드의 난수·티켓 계산을 그대로 구현하고 시드 1·2·3의 완료 시간으로 대조 검증했다. 동일 길이 L의 Stride 교대는 완료가 2L-1, 2L이므로 U=1-1/(2L)다. 다른 티켓 비율, 퀀텀, 새 작업 도착 시 pass 초기화도 추가 실험 대상으로 적절하다.',
  F('lottery','실행 길이와 완료 시간 공정성. Lottery는 시드 0-499 평균, Stride는 이론값.'))

q('10',1,'캐시보다 큰 작업 집합',
  'a:30:200, CPU 1개, 캐시 크기 100이면 <b>30틱</b>이다.',
  '작업 집합 200이 캐시 100에 들어가지 않아 warm 상태가 되지 않는다. 1틱에 남은 실행량이 1씩 줄어든다.')
q('10',2,'캐시를 300으로 늘리기',
  '기본 warmup=10, warm rate=2에서 <b>20틱</b>이다.',
  '첫 10틱은 속도 1로 10만큼 처리하고, 나머지 실행량 20은 속도 2로 10틱에 처리한다. 10+(30-10)/2=20이다. 캐시를 키워도 워밍업 이전부터 빨라지는 것은 아니다.')
q('10',3,'남은 시간 -T의 변화',
  '차가울 때는 1씩, 따뜻해진 뒤에는 2씩 줄어든다.',
  '이 시뮬레이터에서 실행 길이는 실시간 틱 수가 아니라 처리해야 할 일의 양이다. -r=2이면 warm CPU가 한 틱에 처리하는 일의 양이 2가 된다. 마지막 양이 2보다 작으면 마지막 틱을 모두 사용한 것으로 표시할 수 있다.')
q('10',4,'warmup 시간 -w의 영향',
  '캐시 300에서 기본 w=10이면 20틱, w=5이면 18틱, w=20이면 25틱이다.',
  'warmup 실행량을 채운 뒤 캐시 표시가 w가 되고 빠른 실행이 시작된다. 완료 시간은 이 예에서 w+ceil((30-w)/2)로 계산한다. 워밍업이 작업 길이보다 길거나 캐시에 안 들어가면 가속 효과를 얻지 못한다.')
q('10',5,'공유 RR 큐의 2 CPU 일정',
  'a:100:100,b:100:50,c:100:50, 기본 캐시 100에서 <b>150틱</b>이다.',
  '큐를 돌면서 작업이 CPU 사이를 이동하고, 한 CPU에 새 작업이 들어오면 기존 캐시 상태를 밀어낼 수 있다. 이 일정은 각 CPU의 warm 비율 0%다. 총 일 300을 속도 1인 CPU 둘이 처리해 150틱이 걸린다.')
q('10',6,'친화성 지정의 효과',
  '-A a:0,b:1,c:1이면 <b>110틱</b>이다. 반대로 a·b를 CPU0, c를 CPU1에 고정하면 <b>200틱</b>이다.',
  '좋은 배치에서는 CPU0의 a는 55틱에 끝나고, CPU1의 b·c 작업 집합 합 100이 캐시에 들어가 110틱에 끝난다. 나쁜 배치의 a·b 합 150은 캐시 100을 초과해 그 CPU가 200틱을 쓴다. 큐에 작업 수가 비슷해도 작업 집합 크기와 캐시 배치에 따라 결과가 달라진다.')
q('10',7,'초선형 가속',
  '캐시 50: CPU 1·2·3개에서 300·150·100틱. 캐시 100: 300·150·55틱이다.',
  '각 작업의 집합은 100이다. 캐시 100과 CPU 3개면 작업마다 CPU를 하나씩 써서 warm 상태를 유지한다. 단일 CPU 300틱과 비교한 가속은 300/55≈5.45배로 CPU 3개보다 크다. CPU 수뿐 아니라 사용 가능한 캐시와 재사용 패턴도 함께 바뀐 결과다.',
  T(['캐시/CPU','1개','2개','3개'],[[50,300,150,100],[100,300,150,55]]))
q('10',8,'CPU별 큐와 peek 간격',
  '동일 작업과 기본 시드에서 -p의 P=0·1·5·30·100 결과는 200·90·90·100·130틱이다.',
  'P=0은 훔치기를 끄므로 초기 배치의 불균형을 고치지 못한다. 더 자주 살피면 부하를 빠르게 맞추지만 실제 구현에서는 락·탐색·이주 비용이 든다. 이 시뮬레이터는 그 비용 전체를 모델링하지 않으므로 P=1이 실제 시스템에서도 언제나 최선이라고 결론 내리지 않는다.',
  T(['P','총 틱'],[[p,finished(f'10_8_p{p}')] for p in [0,1,5,30,100]]))
q('10',9,'무작위 워크로드 비교',
  '예시 -j 3 -R 40 -W 100 -M 100, 공유 큐와 기본 나머지 설정의 완료 시간은 아래와 같다.',
  '작업 길이, 캐시 적합성, 이동 횟수를 함께 본다. 작업이 3개뿐인 이 예에서 CPU 4개는 추가 유휴 CPU와 다른 이동 패턴 때문에 2개보다 오히려 느릴 수 있다. CPU를 늘리면 언제나 단조롭게 빨라진다는 가정은 성립하지 않는다.',
  T(['시드','CPU1','CPU2','CPU4'],[[s,*[finished(f'10_9_s{s}_n{n}') for n in [1,2,4]]] for s in [1,2,3]]))

q('13',1,'free 매뉴얼과 필드',
  'total은 전체 사용 가능한 메모리, free는 현재 미사용 메모리, available은 스왑 없이 새 작업에 쓸 수 있을 것으로 추정한 메모리다.',
  'free와 available은 다르다. available은 회수 가능한 캐시 등을 고려한다. used·buff/cache·shared·swap도 함께 확인한다. 도구 버전에 따라 used 계산식과 출력이 달라질 수 있어 실제 man free를 기준으로 해석한다. free -m은 MiB 단위, free -h는 읽기 쉬운 단위다.')
q('13',2,'내 시스템의 전체·사용 가능 메모리',
  '숫자는 해당 Linux 시스템에서 free -m 또는 free -h로 측정해야 한다. 이 문서에서는 사용자 시스템의 값을 임의로 넣지 않았다.',
  '설치된 RAM과 total이 정확히 같지 않을 수 있다. 커널·장치 예약과 시스템의 관측 범위를 고려하고, “프로그램이 추가로 쓸 수 있는 양”에는 available을 본다. /proc/meminfo의 MemTotal·MemAvailable과 대조하면 된다.')
q('13',3,'memory-user.c 작성',
  '다음은 입력한 MiB를 할당하고 전체 바이트를 계속 접근하는 프로그램이다. 두 번째 인자로 실행 시간을 제한할 수 있다.',
  'volatile 접근으로 반복문 제거를 막고, 처음 memset으로 실제 페이지에 접근한다. PID를 출력해 pmap에 사용할 수 있다. 크기 계산 오버플로와 malloc 실패를 확인한다. 무한 반복 대신 기본 10초를 사용했다.',
  C('''#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
static double now(void) {
    struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t);
    return t.tv_sec+t.tv_nsec/1e9;
}
int main(int argc,char **argv) {
    if(argc<2) return 1;
    unsigned long long mb=strtoull(argv[1],NULL,10);
    if(mb==0 || mb>SIZE_MAX/(1024u*1024u)) return 1;
    size_t n=(size_t)mb*1024u*1024u;
    double seconds=argc>2 ? atof(argv[2]) : 10;
    if(seconds<=0) return 1;
    unsigned char *p=malloc(n);
    if(!p) { perror("malloc"); return 1; }
    memset(p,0,n);
    volatile unsigned char *a=p;
    printf("pid=%ld bytes=%zu\\n",(long)getpid(),n); fflush(stdout);
    double end=now()+seconds;
    do {
        for(size_t i=0;i<n;i++) a[i]=(unsigned char)(a[i]+1);
    } while(now()<end);
    free(p); return 0;
}''','gcc -O2 memory-user.c -o memory-user / ./memory-user 64 10'))
q('13',4,'실행·종료와 큰 메모리 요청',
  '페이지를 실제로 접근하면 메모리 사용이 늘고 available이 감소한다. 종료하면 그 프로세스의 자원이 회수된다.',
  'malloc 성공만으로 요청한 모든 물리 페이지가 즉시 상주하는 것은 아니다. 매우 큰 요청은 malloc 실패, 스왑 증가·쓰래싱, 메모리 부족으로 인한 종료 등으로 이어질 수 있다. 캐시 회수·다른 프로세스·커널 메타데이터 때문에 free의 변화가 요청량과 바이트 단위로 딱 맞을 필요는 없다.')
q('13',5,'pmap이 보여 주는 것',
  '한 프로세스의 가상 메모리 매핑을 보여 준다. 주소·영역 크기·권한·매핑 파일 등을 확인한다.',
  'pmap PID의 기본 출력과 pmap -x PID의 상세 출력을 비교한다. -X의 추가 열은 커널과 procps 버전에 따라 다를 수 있다. 가상 매핑 크기와 RAM에 상주한 RSS를 구분한다.')
q('13',6,'PID 찾기',
  'ps auxw에서 PID 열을 확인하거나 프로그램이 출력한 getpid() 값을 사용한다.',
  '이름이 같아도 다른 프로세스가 여러 개 있을 수 있다. 정확한 PID를 지정하고 프로그램이 끝나기 전에 pmap을 실행한다. 예: ./memory-user 64 30을 실행하고 다른 터미널에서 pmap -x <출력된 PID>를 실행한다.')
q('13',7,'실제 주소 공간이 복잡한 이유',
  '실행 파일의 코드·데이터, 힙, 스택, 공유 라이브러리, 익명 매핑, 파일 매핑, 로더, 커널 지원 영역 등 여러 매핑이 보인다.',
  '단순한 코드/힙/스택 그림은 개념 모델이다. 권한이 r-x인 코드와 rw인 데이터처럼 같은 파일도 여러 영역으로 나뉠 수 있다. RSS 합은 공유 페이지를 중복해서 세는 등 특성이 있으므로 다른 프로세스와 무조건 합산하지 않는다.')
q('13',8,'크기를 바꾼 pmap 결과',
  '더 큰 할당을 하고 전부 접근하면 관련 익명·힙 매핑의 크기와 RSS가 대체로 증가한다.',
  '작은 할당은 기존 힙 안에서 충족되고 큰 할당은 별도 mmap 영역으로 나타날 수 있다. 할당기의 문턱·재사용·페이지 반올림 때문에 한 매핑이 항상 요청량만큼 정확히 늘어나는 것은 아니다. 아직 접근하지 않은 매핑의 RSS는 가상 크기보다 작을 수 있다.')

q('14',1,'NULL 역참조',
  'C에서 정의되지 않은 동작이다. 일반적인 Linux에서 0번 페이지가 매핑되지 않아 SIGSEGV로 끝나는 모습을 관찰한다.',
  '“반드시 언제나 같은 오류가 난다”는 C 언어의 보장은 아니다. gcc -O0 -g로 아래 예를 빌드해 관찰한다. 최적화·실행 환경에 따라 결과가 달라질 수 있다.',
  C('''#include <stddef.h>
int main(void) {
    volatile int *p=NULL;
    return *p; /* intentionally invalid */
}''','null.c: 의도적으로 잘못된 코드'))
q('14',2,'gdb가 보여 주는 정보',
  'gcc -O0 -g null.c -o null 후 gdb ./null, run을 실행하면 보통 SIGSEGV와 실패한 소스 행을 볼 수 있다.',
  'bt로 호출 스택, print p로 포인터 값을 본다. -g는 이름·행 정보를 제공할 뿐 버그를 고치지 않는다. 디버거를 사용했다고 프로그램의 메모리 접근이 합법적으로 바뀌는 것도 아니다.')
q('14',3,'Valgrind Memcheck의 NULL 보고',
  'valgrind --leak-check=yes ./null은 보통 주소 0의 잘못된 읽기와 그 크기, 발생 위치를 보고한다.',
  '이 문제의 중심은 누수가 아니라 Invalid read다. 누수 점검 옵션이 있어도 Memcheck는 잘못된 접근을 함께 찾는다. SIGSEGV가 발생하면 종료 원인도 보인다. 구체적인 행 번호·메시지는 빌드와 버전에 따라 달라진다.')
q('14',4,'malloc한 뒤 free하지 않기',
  '프로그램은 정상 종료할 수도 있다. gdb가 자동으로 누수 목록을 보여 주지는 않지만 Memcheck는 누수 상태를 추적한다.',
  '종료 때 OS가 메모리를 회수해도 장시간 실행 중의 누수는 문제다. 포인터가 완전히 사라지면 definitely lost, 종료까지 도달 가능한 포인터가 남으면 still reachable로 분류될 수 있다. 따라서 무조건 definitely lost라고 쓰지 않는다.',
  C('''#include <stdlib.h>
int main(void) {
    int *p=malloc(100*sizeof *p);
    if(!p) return 1;
    p[0]=1;
    p=NULL; /* intentionally lose the allocation */
    return 0;
}''','gcc -O0 -g leak.c -o leak / valgrind --leak-check=full ./leak'))
q('14',5,'100개 배열의 data[100]',
  '범위 밖 쓰기다. 유효한 인덱스는 0-99이며 프로그램은 올바르지 않다.',
  '바로 충돌하지 않아도 인접 데이터나 할당기 정보를 망가뜨릴 수 있다. malloc으로 얻은 배열에 쓰면 Memcheck는 블록 끝 바로 뒤의 Invalid write를 보고한다. 원문의 data[I00]은 data[100]으로 읽는다.',
  C('''int *data=malloc(100*sizeof *data);
if(!data) return 1;
data[100]=0; /* intentionally out of bounds */
free(data);''','main 내부의 오류 예시'))
q('14',6,'free 후 배열 접근',
  'use-after-free이며 정의되지 않은 동작이다. 이전 값이 우연히 출력돼도 정답 코드가 아니다.',
  '해제된 공간은 재사용되거나 내용이 달라질 수 있다. Memcheck는 해제된 블록에 대한 Invalid read와 할당·해제 위치를 보여 준다.',
  C('''int *data=malloc(100*sizeof *data);
if(!data) return 1;
data[0]=7;
free(data);
printf("%d\\n",data[0]); /* intentionally invalid */''','main 내부의 오류 예시'))
q('14',7,'배열 중간 포인터를 free',
  'free(data+1)은 잘못된 해제다. 할당 함수가 반환한 시작 포인터 또는 NULL을 전달해야 한다.',
  '보통 할당기 오류로 중단되거나 Memcheck가 Invalid free를 보고하지만, UB이므로 모든 환경에서 같은 결과가 보장되지 않는다. 바로 중단되면 도구 없이 증상을 알 수 있어도 도구는 원인을 찾는 데 도움이 된다.',
  C('''int *data=malloc(100*sizeof *data);
if(!data) return 1;
free(data+1); /* intentionally invalid */''','main 내부의 오류 예시'))
q('14',8,'realloc 기반 벡터',
  '매번 1개씩 늘리기보다 용량을 2배로 늘리면 push가 분할 상환 O(1), 인덱스 접근 O(1)이고 연속 배치로 캐시 사용이 좋다.',
  '재할당 한 번은 복사 때문에 O(n)일 수 있다. 실패하면 기존 블록은 살아 있으므로 임시 포인터로 받고 성공 후 바꾼다. 연결 리스트는 연속 공간이 필요 없고 이미 찾은 위치의 연결 변경이 쉽지만 인덱스 탐색은 O(n), 노드 메타데이터와 지역성 비용이 든다.',
  C('''#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
typedef struct { int *a; size_t n,cap; } Vec;
static int push(Vec *v,int x) {
    if(v->n==v->cap) {
        if(v->cap>SIZE_MAX/2) return -1;
        size_t cap=v->cap ? 2*v->cap : 4;
        if(cap>SIZE_MAX/sizeof *v->a) return -1;
        int *tmp=realloc(v->a,cap*sizeof *v->a);
        if(!tmp) return -1;
        v->a=tmp; v->cap=cap;
    }
    v->a[v->n++]=x; return 0;
}
int main(void) {
    Vec v={0};
    for(int i=0;i<100;i++) {
        if(push(&v,i)<0) { free(v.a); return 1; }
    }
    printf("n=%zu cap=%zu last=%d\\n",v.n,v.cap,v.a[v.n-1]);
    free(v.a); return 0;
}''','vector.c: n=100, cap=128, last=99의 논리적 결과'))
q('14',9,'디버거와 메모리 도구 익히기',
  'gdb는 실패 순간의 실행 상태를, Memcheck는 잘못된 접근·누수의 이력을 찾는 데 적합하다.',
  '재현 입력을 고정하고 -g로 빌드 → gdb run/bt/print → valgrind --leak-check=full --track-origins=yes → 수정 후 같은 입력으로 재확인한다. 정상 실행만으로 메모리 안전성을 증명할 수는 없다. 이 과제는 고정 숫자 답이 없는 실습이다.')

relocrows=[]
for s in [1,2,3]:
    t=runs(f'15_1_s{s}')
    base=re.search(r'Base\s+:.*decimal (\d+)',t).group(1)
    limit=re.search(r'Limit\s+: (\d+)',t).group(1)
    for i,va,res in re.findall(r'VA\s+(\d+):.*?\(decimal:\s*(\d+)\) --> (.*)',t):
        pa=re.search(r'decimal: (\d+)',res)
        relocrows.append([s,base,limit,va,pa.group(1) if pa else '위반'])
q('15',1,'시드 1·2·3 주소 변환',
  'VA&lt;limit이면 PA=base+VA, 그 외에는 위반이다. 기본 A=1024, 물리 메모리 P=16384의 결과는 아래와 같다.',
  '유효 주소의 끝은 limit-1이다. 같은 시드에서도 base·limit을 직접 지정하면 난수 소비 순서가 달라져 이후 VA열이 달라질 수 있으므로 생성된 주소를 반드시 확인한다.',
  T(['시드','base','limit','VA','PA/결과'],relocrows,[.10,.20,.17,.18,.35]))
q('15',2,'모든 생성 주소를 유효하게 만드는 limit',
  '이 조건의 최소 limit은 <b>930</b>이다. 가장 큰 생성 주소 929를 포함하려면 1을 더해야 한다.',
  '기본 -s 0 -n 10의 VA는 430,265,523,414,802,310,488,597,929,516이다. -l 930을 명시하면 난수 소비가 달라져 앞에 776이 추가되는 등 주소열이 바뀌지만 여전히 최대는 929다. -l 929이면 929는 위반이다. 모든 가능한 0-1023 주소를 보장하려면 limit=1024를 써야 한다.')
q('15',3,'base 최댓값: 원문 번역 차이',
  '영문 원문은 <b>base의 최댓값</b>을 묻는다. limit=100, P=16384이면 base≤P-limit이므로 <b>16284</b>다.',
  'base=16284는 물리 주소 16284-16383에 100바이트 보호 구간을 놓는다. 16285는 끝이 넘쳐 프로그램이 오류를 낸다. 첨부 한국어의 “바운드 레지스터 최댓값”은 영문의 base와 다르다. 한국어를 문자 그대로 읽어 기본 base=2201을 고정하면 물리적으로 가능한 limit은 14183이지만, 논리적 주소 공간 A=1024 안에서는 limit=1024면 이미 모든 주소를 포함한다. 1024바이트 전체 슬롯 배치를 별도로 요구하면 base≤15360이다. 이 답은 시뮬레이터가 검사하는 base+limit≤P 조건을 기준으로 한다.')
q('15',4,'큰 주소 공간으로 재실행',
  '예: -s 1 -n 10 -a 4k -p 64k -b 32768 -l 4k -c. 생성 주소 10개가 모두 유효하다.',
  '전체 VA 범위 0-4095가 limit=4096 안에 들어간다. VA=550이면 PA=33318, VA=3471이면 PA=36239다. 물리 공간 조건도 32768+4096≤65536으로 만족한다. 크기를 바꿔도 범위 검사와 더하기 원리는 같다.')
q('15',5,'limit에 따른 유효 주소 비율',
  'VA가 0…A-1에서 균등하면 P(valid)=min(limit,A)/A다. 0≤limit≤A에서는 직선 limit/A다.',
  '아래는 A=1024, base=0, 시드 0의 VA 1000개를 고정해 측정한 표본 비율과 이론값이다. 난수·base·주소열을 고정해야 limit의 효과만 비교할 수 있다. 유한 표본은 직선 근처에서 흔들린다. 원점과 limit=A의 끝점은 각각 0과 1이다.',
  F('relocation','유효 주소 비율: 이론과 고정된 시드 0의 1000개 주소.'))

segrows=[]
for s in [0,1,2]:
    for i,va,res in re.findall(r'VA\s+(\d+):.*?\(decimal:\s*(\d+)\) --> (.*)',runs(f'16_1_s{s}')):
        pa=re.search(r'decimal:\s*(\d+)',res)
        segrows.append([s,va,'0' if int(va)<64 else '1',pa.group(1) if pa else '위반'])
q('16',1,'128바이트 주소 공간의 두 세그먼트',
  'SEG0은 VA 0-19에서 PA=VA, SEG1은 VA 108-127에서 PA=512+(VA-128)이다.',
  'VA 상위 비트로 두 영역을 구분한다. 두 번째 세그먼트의 base=512는 물리 구간 끝의 다음 주소를 가리키므로 음수 오프셋을 더한다. 108의 오프셋 -20도 유효해 PA=492다.',
  T(['시드','VA','세그먼트','PA/결과'],segrows,[.13,.24,.23,.4]))
q('16',2,'경계 주소와 -A 검사',
  'SEG0의 최대 유효 VA=<b>19</b>, SEG1의 최소 유효 VA=<b>108</b>. 잘못된 VA의 최소·최대는 <b>20·107</b>이다.',
  '-A 19,20,107,108,127 -c로 유효·위반·위반·유효·유효를 확인한다. 전체 공간을 보려면 -A에 0…127을 명시한다. 역방향 세그먼트의 조건은 -L≤음수 offset&lt;0이며, 정방향의 0≤offset&lt;l과 경계 표현이 다르다.')
q('16',3,'처음·끝 두 주소만 유효하게 만들기',
  '예: --b0 0 --l0 2 --b1 128 --l1 2.',
  'VA 0·1은 PA 0·1로, VA 14·15는 PA 126·127로 변환되고 VA 2-13은 위반이다. 베이스 위치는 두 물리 구간이 합법적이고 겹치지 않도록 다른 값을 선택해도 된다. 유효 여부 패턴을 결정하는 핵심은 두 limit=2다.')
q('16',4,'약 90%가 유효한 설정',
  '겹치지 않는 두 유효 가상 구간의 크기 합을 주소 공간의 약 90%로 잡는다. 예: A=1000, l0=l1=450.',
  '-a 1000 -p 4096 --b0 0 --l0 450 --b1 4096 --l1 450이면 이론상 900/1000=90%다. 기본 시드 0의 1000개 주소 검산은 903개 유효, 97개 위반이다. 유한 표본이 정확히 90%일 필요는 없다. limit은 각 절반 크기 이하여야 하고 물리 구간도 겹치지 않아야 한다.')
q('16',5,'모든 주소를 무효로 만들기',
  '두 limit을 0으로 설정하면 된다: --l0 0 --l1 0.',
  '-a 16 -p 128 --b0 0 --b1 128과 함께 VA 0-15를 모두 지정하면 전부 위반이다. 유효한 길이가 0이므로 어떤 주소도 들어갈 수 없다. 베이스만 엉뚱한 값으로 놓아 설정 자체를 실패시키는 것과 구분한다.')

def malloc_trace(key):
    t=runs(key); rows=[]; op=''
    for line in t.splitlines():
        if line.startswith('ptr['): op=line.split(' returned')[0];ret=re.search(r'returned (-?\d+)',line).group(1)
        elif line.startswith('Free(ptr'): op=line;ret='0'
        elif line.startswith('Free List'):
            pairs=re.findall(r'addr:(\d+) sz:(\d+)',line)
            rows.append([op,ret,' '.join(f'({a},{s})' for a,s in pairs) or '없음'])
    return rows
q('17',1,'BEST의 10개 연산 추적',
  '기본 힙 100바이트, base=1000, 헤더 0, 병합 없음의 결과는 아래와 같다. 빈 목록은 (주소,크기)로 표시했다.',
  '할당은 해당 크기를 떼어 시작 주소를 반환하고, 정상 free는 0을 반환한다. 사용량이 줄어도 병합하지 않아 이웃한 빈 블록이 별개의 원소로 남는다. 외부 단편화가 커지면 빈 공간 합계가 충분해도 큰 요청을 만족시키기 어렵다.',
  T(['연산','반환','연산 후 빈 목록'],malloc_trace('17_short_BEST_ADDRSORT'),[.31,.12,.57]))
q('17',2,'WORST로 바꾸기',
  '처음 세 번의 할당·해제는 같지만, 네 번째 Alloc(8)은 1008 대신 가장 큰 블록의 1016을 고른다.',
  '뒤의 Alloc(2)·Alloc(7)은 1024·1026을 반환한다. 최종 빈 목록은 (1000,3),(1003,5),(1008,8),(1016,8),(1033,67)이다. 6번의 할당에서 총 20개 원소를 검사한다. 가장 큰 구간을 쓰므로 항상 단편화가 줄어드는 것은 아니다.')
q('17',3,'FIRST와 탐색 비용',
  'ADDRSORT의 FIRST는 이 예에서 BEST와 같은 주소를 반환하되 검사 수가 1·2·3·3·1·3, 합 <b>13</b>으로 줄어든다. BEST는 18이다.',
  'FIRST는 맞는 원소를 만나면 중단한다. BEST는 더 작은 적합 후보를 찾으려고 끝까지 본다. 일반적으로 두 정책의 반환 주소가 항상 같지는 않으며, 이 짧은 예제에서만 같은 결과다.')
q('17',4,'정렬 순서와 정책',
  '이 구현에서 BEST·WORST는 정렬되어 있어도 전체를 검색한다. FIRST는 리스트 순서의 영향을 직접 받는다.',
  'FIRST+SIZESORT-는 이 예의 6개 할당에서 모두 첫 원소가 맞아 총 6번 검사한다. 주소순·크기 오름차순은 총 13번이다. 큰 블록을 먼저 쓰는 동작은 WORST와 비슷해지지만 탐색 방식은 다르다. 같은 크기의 동률 선택과 반환 주소도 정렬에 따라 달라질 수 있다.',
  T(['정책','ADDRSORT','SIZESORT+','SIZESORT-'],[[p,*[sum(map(int,re.findall(r'searched (\d+)',runs(f'17_short_{p}_{o}')))) for o in ['ADDRSORT','SIZESORT+','SIZESORT-']]] for p in ['BEST','WORST','FIRST']]))
longrows=[]
for order in ['ADDRSORT','SIZESORT+','SIZESORT-']:
    for c in [False,True]:
        t=runs(f'17_long_{order}_{c}')
        longrows.append([order,'있음' if c else '없음',re.findall(r'Free List \[ Size (\d+) \]',t)[-1],len(re.findall(r'returned -1',t))])
q('17',5,'1000개 연산과 병합',
  'BEST·시드 0·헤더 0·할당 비율 50%에서 병합과 정렬별 결과는 아래와 같다.',
  '주소순 병합은 단편 수와 실패 횟수를 크게 줄인다. 이 시뮬레이터의 병합은 리스트에서 연속한 원소를 검사하므로 크기순 목록에서는 물리적으로 인접한 청크가 떨어져 있어 잘 합쳐지지 않는다. 이는 이 구현의 한계이며, 모든 실제 할당기가 병합을 위해 주소순 단일 리스트를 써야 한다는 뜻은 아니다.',
  T(['순서','병합','최종 빈 블록 수','할당 실패'],longrows,[.31,.17,.26,.26]))
q('17',6,'할당 비율 P의 영향',
  '할당 비율이 높으면 사용 중인 블록이 늘어 공간 부족 실패가 많아진다. 낮으면 해제가 우세해 빈 공간이 유지된다.',
  '같은 1000개 연산, BEST·시드 0·ADDRSORT·병합에서 P=10·50·75·100의 실패는 0·29·456·983이다. P=100에서는 한 번도 해제하지 않아 대부분 요청이 실패한다. 이 프로그램은 P=0을 assertion으로 거부하므로 0에 가까운 효과는 P=1·10 등으로 관찰한다. “초기부터 해제만”은 해제할 블록이 없다는 점도 고려한다.')
q('17',7,'심한 외부 단편화 만들기',
  '100바이트를 10바이트 블록 10개로 채운 뒤 짝수 번호 블록만 해제하고 15바이트를 요청한다.',
  '빈 합계 50바이트지만 최대 연속 빈 구간은 10이므로 마지막 요청은 -1이다. 사용 중인 블록이 사이에 있어 병합을 켜도 15바이트 구간이 생기지 않는다. 모두 같은 크기라 BEST·WORST·FIRST의 마지막 실패는 같다.',
  C('''malloc.py -S 100 -H 0 \
  -A +10,+10,+10,+10,+10,+10,+10,+10,+10,+10,-0,-2,-4,-6,-8,+15 -c''','시뮬레이터 명령: 필요하면 한 줄로 입력'))

q('18',1,'주소 공간·페이지 크기와 선형 테이블',
  'PTE를 4바이트로 가정하면 테이블 크기는 (A/P)×4다. 주소 공간은 비례, 페이지 크기는 반비례로 영향을 준다.',
  '큰 페이지는 PTE 수와 TLB 부담을 줄이지만 내부 단편화, 큰 할당 단위, 필요한 일부만 접근할 때의 I/O·메모리 낭비가 커질 수 있다. 물리 메모리 512 MiB 자체가 아래 가상 페이지 수를 바꾸지는 않는다.',
  T(['A','P','항목 수','테이블 크기'],[
   ['1 MiB','1 KiB',1024,'4 KiB'],['2 MiB','1 KiB',2048,'8 KiB'],['4 MiB','1 KiB',4096,'16 KiB'],
   ['1 MiB','2 KiB',512,'2 KiB'],['1 MiB','4 KiB',256,'1 KiB']]))
pagerows=[]
for u in [0,25,50,75,100]:
    t=runs(f'18_2_u{u}')
    pagerows.append([u,len(re.findall(r'^\s*\[\s*\d+\]\s+0x8',t,re.M)),len(re.findall(r'--> (?! Invalid)',t)),5-len(re.findall(r'--> (?! Invalid)',t))])
q('18',2,'할당 비율 -u',
  '높일수록 유효 PTE가 많아져 무효 주소를 만날 가능성이 줄어든다. 시드 0의 결과는 아래와 같다.',
  '-u는 페이지별 할당 확률이다. 16개라는 짧은 표본에서 25%가 정확히 4페이지가 되는 것은 아니다. 테이블은 항상 16개 항목을 가져 크기는 64바이트로 같다. 변환은 VPN=floor(VA/1024), offset=VA mod 1024, PA=PFN×1024+offset으로 한다.',
  T(['u(%)','유효 PTE/16','유효 참조/5','무효 참조/5'],pagerows))
randompagerows=[]
for i in [1,2,3]:
    t=runs(f'18_3_{i}')
    for va,result in re.findall(r'VA (0x[0-9a-f]+).*? -->\s*(.*)',t):
        randompagerows.append([i,va,result.strip()])
q('18',3,'세 가지 무작위 배치의 변환',
  '원문에 지정된 시드·크기·기본 u=50을 사용한 각 VA의 결과는 아래와 같다.',
  '① 8바이트 페이지·32바이트 주소 공간은 구조를 보여 주는 극단적인 장난감 예다. 너무 작은 페이지는 테이블·TLB 관리 부담이 크다. ② 8 KiB, ③ 1 MiB는 더 큰 페이지의 절충을 관찰하는 예이며, 1 MiB라는 크기만으로 주소 변환 원리상 불가능한 것은 아니다. 실제 지원 크기는 아키텍처에 따른다.',
  T(['조건','VA','PA 또는 무효 이유'],randompagerows,[.10,.26,.64]))
q('18',4,'시뮬레이터의 제약',
  '이 선형 변환 시뮬레이터는 물리 메모리가 주소 공간보다 커야 한다. A=64 KiB, P_phys=32 KiB는 u=25라도 설정 오류로 끝난다.',
  '이것은 도구의 단순화 제약이다. 실제 가상 메모리 시스템은 부분 상주·스왑 등으로 가상 공간이 RAM보다 클 수 있다. 페이지·주소 공간·물리 메모리의 크기는 도구의 2의 거듭제곱 및 정수 페이지 배치 조건을 만족해야 한다. 예: 페이지 3 KiB와 주소 공간 16 KiB는 정수배가 아니어서 거부된다.')

q('19',1,'타이머 정확도와 측정 길이',
  '환경마다 다르다. 타이머 분해능을 먼저 확인하고, 총 측정 시간이 오차·조회 비용보다 충분히 길도록 반복 수를 늘린다.',
  'gettimeofday는 μs를 출력하지만 그 자체가 정확도 보장은 아니다. 예컨대 전체 구간을 100 ms 이상으로 늘리고 반복 측정의 중앙값·변동폭을 비교할 수 있다. 100 ms는 보편적인 정답 상수가 아니라 재현을 위한 시작점이다. CPU 주파수·부하·가상 머신도 기록한다.')
q('19',2,'tlb.c: 페이지당 하나씩 접근',
  '페이지 수와 반복 횟수를 인자로 받고, 페이지마다 한 바이트를 갱신해 평균 ns/access를 계산한다.',
  '메모리는 측정 전에 처음 접근하고 워밍업한다. 아래 gettimeofday 기반 예에서는 총 구간을 충분히 길게 해야 한다. volatile 접근으로 최적화 제거를 막았다. 데이터 캐시의 충돌·대형 페이지 등도 지연에 영향을 줄 수 있으므로 관찰된 계단을 TLB 크기만으로 단정하지 않는다.',
  C('''#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
static double ns(void) {
    struct timeval t; gettimeofday(&t,NULL);
    return t.tv_sec*1e9+t.tv_usec*1000.0;
}
int main(int argc,char **argv) {
    if(argc!=3) return 1;
    size_t pages=strtoull(argv[1],NULL,10);
    size_t reps=strtoull(argv[2],NULL,10);
    long psz=sysconf(_SC_PAGESIZE);
    if(!pages || !reps || psz<=0 || pages>SIZE_MAX/(size_t)psz)
        return 1;
    size_t step=(size_t)psz, bytes=pages*step;
    unsigned char *p=calloc(1,bytes);
    if(!p) return 1;
    volatile unsigned char *a=p;
    for(size_t j=0;j<pages;j++) a[j*step]=1;
    for(int warm=0;warm<10;warm++)
        for(size_t j=0;j<pages;j++) a[j*step]++;
    double t=ns();
    for(size_t r=0;r<reps;r++)
        for(size_t j=0;j<pages;j++) a[j*step]++;
    double elapsed=ns()-t;
    printf("%zu,%zu,%.3f\\n",pages,reps,
           elapsed/((double)pages*reps));
    free(p); return 0;
}''','gcc -O2 tlb.c -o tlb / ./tlb 64 100000'))
q('19',3,'페이지 수·반복 횟수 바꾸기',
  '페이지 수를 1,2,4,…,4096으로 바꾸고 각 조건을 여러 번 실행한다. 고정된 최소 시행 횟수의 정답은 없다.',
  '예시 스크립트는 20회 시행해 중앙값을 계산할 수 있는 CSV를 만든다. 반복 횟수는 한 번의 측정이 충분히 긴지 확인한 뒤 조정한다. 중앙값과 IQR·오차 막대가 안정되는지도 본다. 페이지 수가 늘면 같은 reps의 총 실행시간도 늘므로 모든 조건의 정밀도가 같다고 가정하지 않는다.',
  C('''import subprocess
print("pages,reps,ns_access")
for power in range(13):
    pages=2**power
    for trial in range(20):
        line=subprocess.check_output(
            ["./tlb",str(pages),"100000"],text=True)
        print(line.strip())''','python3 sweep-tlb.py > tlb.csv'))
q('19',4,'그래프로 확인하기',
  'x축은 페이지 수(log2), y축은 페이지당 접근 시간(ns)으로 두고 중앙값과 변동폭을 그린다.',
  '작은 페이지 수에서 낮은 지연, 특정 범위를 넘어선 지연 상승이 보이면 캐시·TLB 용량과 연관해 분석할 수 있다. 아래는 실제로 저장한 CSV를 그리는 코드다. 이 Windows 환경에서 얻은 사용자 Linux 하드웨어 측정값은 제공하지 않는다.',
  C('''import csv,statistics
import matplotlib.pyplot as plt
groups={}
with open("tlb.csv") as f:
    for r in csv.DictReader(f):
        groups.setdefault(int(r["pages"]),[]).append(float(r["ns_access"]))
x=sorted(groups)
y=[statistics.median(groups[n]) for n in x]
plt.plot(x,y,"o-"); plt.xscale("log",base=2)
plt.xlabel("Pages"); plt.ylabel("ns/access")
plt.grid(True); plt.savefig("tlb-measured.png",dpi=200)''','실측 CSV용 그래프 템플릿; 측정 결과를 만들어 넣는 코드가 아니다'))
q('19',5,'최적화에 의한 반복 제거 방지',
  'volatile 메모리 접근 또는 결과를 관찰 가능한 형태로 사용해 반복문이 살아 있도록 한다.',
  '2번 예는 volatile 포인터로 쓰기를 수행한다. 단순히 지역 변수를 증가시키고 한 번도 쓰지 않으면 컴파일러가 루프를 제거할 수 있다. 최적화 옵션만 낮추면 다른 실행 비용도 바뀌므로 측정 목적에 맞는 코드를 먼저 만든다.')
q('19',6,'한 CPU에 고정하기',
  'Linux의 sched_setaffinity 또는 taskset으로 허용된 CPU 하나에 고정한다.',
  '먼저 taskset -pc PID나 시스템의 허용 CPU 목록을 확인한다. 예: taskset -c <허용된 CPU 번호> python3 sweep-tlb.py를 실행하면 자식도 같은 친화성을 물려받는다. 이동이 생기면 각 CPU의 TLB·캐시가 다시 채워지고 잡음이 증가한다. NUMA 메모리 위치와 다른 부하도 통제한다.')
q('19',7,'초기화·첫 접근 비용',
  '처음 접근하는 페이지의 요구 할당·0 초기화·페이지 폴트가 TLB 미스 비용과 섞인다. 측정 전에 모든 페이지를 쓰고 워밍업한다.',
  'calloc 호출만으로 물리 페이지가 모두 상주했다고 단정할 수 없다. 2번의 페이지별 쓰기는 실물 페이지를 준비하는 데 도움이 된다. 첫 실행(cold)과 워밍업 뒤(steady-state)를 구분해서 보고하면 목적에 맞는 비교가 된다. 워밍업으로 전체가 TLB에 들어간다는 보장은 없다.')

multilevelrows=[]
for s in [0,1,2]:
    for block in runs(f'20_2_s{s}').split('Virtual Address ')[1:]:
        va=block.split(':',1)[0]
        pde=re.search(r'pde index:(0x[0-9a-f]+)',block).group(1)
        pte=re.search(r'pte index:(0x[0-9a-f]+)',block)
        pa=re.search(r'Translates to Physical Address (0x[0-9a-f]+)',block)
        result=pa.group(1) if pa else ('PDE 무효' if 'page directory entry not valid' in block else 'PTE 무효')
        refs=3 if pa else (2 if pte else 1)
        multilevelrows.append([s,va,pde,pte.group(1) if pte else '-',result,refs])
q('20',1,'단계 수와 루트 레지스터 수',
  '선형·2단계·3단계 모두 <b>최상위 테이블을 가리키는 루트 레지스터 1개</b>면 된다.',
  '하위 테이블의 위치는 상위 항목이 제공한다. 단계마다 전용 루트 레지스터가 추가되는 것은 아니다. 페이지 크기·형식이 정해져 있다는 전제이며, 전체 MMU에 레지스터가 딱 하나만 있다는 뜻은 아니다.')
q('20',2,'시드 0·1·2 다단계 변환',
  '기본 모델은 VA 15비트, 페이지 32바이트다. VA=PDE 인덱스 5비트 | PTE 인덱스 5비트 | offset 5비트다. 모든 주소의 검산 결과는 아래와 같다.',
  '예: 시드 0의 VA=0x611c는 PDE=24, PTE=8, offset=28이다. PTE PFN=53이므로 PA=53×32+28=0x6bc다. 마지막 열은 캐시·TLB 효과를 제외하고 실제 데이터까지 읽는 전체 메모리 참조 수다. 성공은 PDE+PTE+데이터로 3회, PDE 무효는 1회, PTE 무효는 2회다. 순수 변환의 테이블 접근만 세면 성공 시 2회다.',
  T(['시드','VA','PDE','PTE','결과','참조'],multilevelrows,[.07,.18,.13,.13,.35,.14]))
q('20',3,'캐시에 의한 테이블 접근 가속',
  '같은 주소 영역을 반복 접근하면 상위 테이블과 인접 PTE에 캐시 히트가 생기기 쉽다. 무작위·넓은 접근은 불리할 수 있다.',
  '다수 VA가 같은 PDE·테이블 페이지를 공유해 상위 단계에 재사용이 많다. 한 캐시 라인에 여러 PTE가 들어가 공간 지역성도 생긴다. 실제 비용은 각 단계가 반드시 RAM까지 가는 n번의 고정 비용과 같지 않다. TLB 히트는 테이블 탐색 자체를 건너뛴다.')

q('21',1,'vmstat의 CPU 사용량',
  'mem 1은 계산을 반복하므로 us가 증가한다. CPU가 N개인 시스템에서 한 단일 스레드가 한 코어를 꽉 쓰면 전체 CPU 기준 약 100/N%일 수 있다.',
  '여러 mem을 실행하면 코어 수까지 계산량이 분산되고 그 뒤에는 CPU를 공유한다. vmstat 1의 첫 CPU 보고는 부팅 이후 평균이므로 이후 구간을 보거나 -y로 첫 보고를 생략한다. 한 프로세스의 top 수치와 전체 CPU 평균을 같은 분모로 읽지 않는다.')
q('21',2,'swpd·free와 프로그램 종료',
  '실제로 1024 MiB를 접근하면 여유 메모리가 대체로 줄고 종료 후 회수된다. swpd는 사용 중인 스왑 용량이다.',
  'RAM 여유가 충분하면 swpd가 그대로일 수 있다. free는 사용하지 않는 RAM이고 캐시 회수·다른 작업·커널 비용 때문에 요청량과 변화량이 딱 맞지는 않는다. available도 함께 본다. 종료한 프로그램의 스왑은 회수되지만 다른 작업의 스왑 상태가 즉시 모두 사라질 필요는 없다.')
q('21',3,'si·so와 RAM을 넘어선 작업 집합',
  'si는 초당 스왑 반입량, so는 초당 스왑 반출량이다. 작업 집합이 상주 가능한 RAM에 들어가면 워밍업 뒤 둘 다 거의 0일 수 있다.',
  '지속적으로 넘치면 재접근할 페이지를 다시 읽고 다른 페이지를 내보내야 해 두 값이 반복적으로 커진다. 사용 중인 RAM·캐시·다른 프로세스·회수 정책에 따라 경계가 달라져 8 GiB 시스템에서 반드시 5000부터 스왑한다고 답할 수는 없다. 구간별 si·so에 시간 간격을 곱해 합산하면 누적량을 추정한다. swpd는 용량, si/so는 전송률이다.')
q('21',4,'CPU·블록 I/O의 변화',
  'RAM에 들어가면 계산 중심이고, 스왑 압박이 커지면 블록 I/O와 대기·커널 처리의 비중이 증가할 수 있다.',
  'bi/bo는 블록 장치 전송률이며 si/so의 스왑만을 뜻하지 않는다. us·sy·wa·id와 함께 패턴을 확인한다. SSD·다른 실행 작업에 따라 CPU가 I/O를 기다리는 모습이 달라지므로 하나의 고정 퍼센트를 정답으로 쓰지 않는다.')
q('21',5,'첫 루프·상주 루프·스와핑 루프의 성능',
  '대역폭=접근한 MiB/걸린 초다. 모든 데이터가 RAM에 있으면 이후 루프가 보통 안정되고, 계속 스와핑하면 시간 증가·대역폭 저하가 나타난다.',
  '첫 루프에는 페이지 준비·초기화 비용이 섞인다. 순환 스캔의 작업 집합이 RAM보다 크면 이후 루프도 지속적으로 반입·반출할 수 있다. 크기별 첫 루프와 이후 루프의 중앙값을 따로 그린다. 제공 mem.c는 미초기화 int 증가와 장기 실행의 signed overflow를 주의해야 하므로, 재현 코드는 13장처럼 초기화된 unsigned·volatile 접근으로 구성하면 해석이 명확하다. 실제 ms·MiB/s는 이 문서에서 실측한 값이 없다.',
  C('''/* with initialized volatile unsigned char *a and size n */
for (int loop=0;loop<10;loop++) {
    double begin=now();
    for(size_t i=0;i<n;i++) a[i]=(unsigned char)(a[i]+1);
    double sec=now()-begin;
    printf("%d,%.6f,%.3f\\n",loop,sec,
           (n/(1024.0*1024.0))/sec);
}''','13장 memory-user.c의 반복 부분을 바꾸는 측정 예시'))
q('21',6,'스왑 공간의 한계와 할당 실패',
  'swapon --show 또는 swapon -s로 확인한다. 한계에 접근하면 할당 실패 또는 실행 중 메모리 부족 종료 등이 발생할 수 있다.',
  'malloc 실패 시점은 단순히 RAM+swap 합계와 같지 않다. overcommit 정책, 프로세스·컨테이너 제한, 다른 작업, 예약량에 의존한다. malloc이 성공해도 페이지를 실제로 쓸 때 OOM이 발생할 수 있다. mem 크기를 끝없이 키워야만 개념을 이해할 수 있는 것은 아니며, 이 문서는 사용자 기기의 메모리를 고갈시키는 실험을 수행하지 않았다.')
q('21',7,'HDD·SSD·RAID 스왑 비교',
  '같은 작업 집합·RAM·CPU·요청 패턴에서 장치별 루프 시간과 유효 대역폭을 비교한다. 정확한 배수는 실제 측정으로 정해야 한다.',
  'SSD는 HDD의 탐색·회전 대기보다 빠른 접근을 제공할 수 있고 RAID는 구성·요청 병렬성에 따라 달라진다. 스왑에는 폴트·회수·I/O·스케줄링 비용도 들어가므로 순수 장치 대역폭만으로 성능을 결정하지 않는다. swapon/swapoff를 통한 장치 변경은 별도의 시스템 실습 단계다. 이 해설에서는 사용자의 스왑 구성을 바꾸지 않았다.')

policyrows=[]
for s in [0,1,2]:
    seq=[a for a,_ in re.findall(r'Access: (\d+)  (HIT|MISS)',runs(f'22_1_s{s}_FIFO'))]
    for p in ['FIFO','LRU','OPT']:
        hm=[('H' if x=='HIT' else 'M') for _,x in re.findall(r'Access: (\d+)  (HIT|MISS)',runs(f'22_1_s{s}_{p}'))]
        policyrows.append([s,p,' '.join(seq),' '.join(hm),hm.count('H')])
q('22',1,'FIFO·LRU·OPT의 전체 히트/미스열',
  '기본 캐시 크기 3, 주소 번호 0-9, 처음 빈 캐시다. H=히트, M=미스이며 각 열은 같은 위치의 주소에 대응한다.',
  'FIFO는 삽입 순서, LRU는 최근 사용 순서, OPT는 미래의 다음 사용 시점으로 피해 페이지를 선택한다. 처음 읽는 페이지는 cold miss다. 이 10개짜리 예에서 시드 2는 세 정책의 히트 수가 우연히 같다.',
  T(['시드','정책','주소열','H/M열','히트'],policyrows,[.07,.12,.37,.35,.09]))
q('22',2,'캐시 5의 최악 참조열',
  'FIFO·LRU: 0,1,2,3,4,5를 반복하면 매번 미스다. MRU: 0,1,2,3,4를 채운 뒤 5,4,5,4…를 반복하면 매번 미스다.',
  'FIFO·LRU는 6개 순환 작업 집합이 5개 프레임을 계속 밀어낸다. MRU는 가장 최근 페이지 4를 5가 내보내고, 4가 다시 5를 내보낸다. 세 경우 모두 캐시를 6으로 키우면 6개 페이지의 최초 미스 뒤 반복 접근은 히트여서 OPT와 같다. 일반 워크로드의 필요 크기를 항상 6이라고 할 수는 없다.')
uniform=[]
for p in ['FIFO','LRU','RAND','CLOCK','OPT']:
    vals=[float(re.search(r'hitrate\s+([\d.]+)',runs(f'22_3_s{s}_{p}')).group(1)) for s in [0,1,2]]
    uniform.append([p,*[f'{v:.2f}%' for v in vals],f'{sum(vals)/3:.2f}%'])
q('22',3,'지역성 없는 무작위 참조',
  '독립·균등한 M개 페이지와 크기 C의 가득 찬 캐시라면 과거 이력만 쓰는 정책의 기대 히트율은 C/M이다.',
  '0-9 중 균등한 주소 1000개, 캐시 5에서 구현 가능한 정책은 약 50%에 모인다. OPT는 미래 정보를 사용하므로 같은 한계를 그대로 적용하지 않으며 약 72%를 얻었다. 원문 보충 255쪽의 “현실적으로 구현 가능한 정책”이라는 한정이 중요하다.',
  T(['정책','시드0','시드1','시드2','평균'],uniform))
extra_results={r['key']:r for r in json.loads((ROOT/'extra_results.json').read_text(encoding='utf-8'))}
localrows=[]
for p,bits in [('FIFO',2),('LRU',2),('RAND',2),('CLOCK',1),('CLOCK',2),('CLOCK',4),('CLOCK',8),('OPT',2)]:
    t=extra_results[f'22_4_{p}_{bits}']['stdout']
    hit,miss,rate=re.search(r'FINALSTATS hits (\d+)\s+misses (\d+)\s+hitrate ([\d.]+)',t).groups()
    localrows.append([p,str(bits) if p=='CLOCK' else '-',hit,miss,rate+'%'])
q('22',4,'지역성·CLOCK 카운터 실험',
  '시드 2026으로 90%는 페이지 0-2, 10%는 3-19를 고른 3000개 참조, 캐시 5를 사용했다. 검산 결과는 아래와 같다.',
  '이 추적에서는 LRU가 RAND보다 약 6.2%p 높다. 첨부의 “RAND가 LRU보다 얼마나 좋은가”는 보충 256쪽에서 비교 방향을 고친 부분이다. CLOCK의 -b는 이 도구에서 참조 카운터의 최대값이지 b비트 하드웨어 필드라는 뜻이 아니다. 카운터가 크면 인기 페이지를 더 오래 보호할 수 있지만 항상 더 좋은 것은 아니다.',
  T(['정책','b','히트','미스','히트율'],localrows))
q('22',5,'실제 응용 프로그램 참조열과 작업 집합',
  'Lackey의 메모리 추적을 페이지 번호로 바꿔 캐시 크기별 히트율을 그린다. 실제 앱의 필요한 크기는 해당 참조열과 목표 히트율에 따라 정한다.',
  '4 KiB 페이지이면 VPN=floor(VA/4096)=VA&gt;&gt;12다. 경계를 넘는 접근은 닿는 모든 페이지를 포함한다. 아래 파서는 I/L/S/M을 처리하고 M을 읽기+쓰기로 두 번 기록한다. 생성 파일을 paging-policy.py -f pages.txt -C 크기 -p LRU -c에 넣는다. 그래프는 이 문서에서 만든 합성 지역성 참조열의 예이며 실제 ls 측정 결과가 아니다.',
  C('''# valgrind --tool=lackey --trace-mem=yes ls 2> trace.txt
import re
with open("trace.txt") as f, open("pages.txt","w") as out:
    for line in f:
        m=re.search(r"\\b([ILSM])\\s+([0-9a-fA-F]+),(\\d+)",line)
        if not m: continue
        op,addr,size=m.groups(); addr=int(addr,16); size=int(size)
        if size==0: continue
        for repeat in range(2 if op=="M" else 1):
            for vpn in range(addr//4096,(addr+size-1)//4096+1):
                out.write(str(vpn)+"\\n")''','Lackey 참조를 가상 페이지 번호로 변환'),
  F('working_set','캐시 크기와 LRU 히트율: 시드 2026의 합성 지역성 참조열 3000개.'))

for ch in CHAPTERS:
    assert len(ANSWERS[ch['id']])==ch['count'], (ch['id'],len(ANSWERS[ch['id']]),ch['count'])
assert sum(len(v) for v in ANSWERS.values())==105
