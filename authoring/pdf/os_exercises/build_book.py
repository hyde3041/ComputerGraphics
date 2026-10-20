from pathlib import Path
from io import BytesIO
import copy, json, math, re
from xml.sax.saxutils import escape

from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.platypus import BaseDocTemplate, Frame, PageTemplate, Paragraph, Spacer, Table, TableStyle, Preformatted, PageBreak, Flowable
from reportlab.pdfgen.canvas import Canvas
from reportlab.graphics.shapes import Drawing, String, Line
from reportlab.graphics.charts.lineplots import LinePlot
from reportlab.graphics import renderPDF
from pypdf import PdfReader, PdfWriter, PageObject, Transformation
from pypdf.generic import RectangleObject, ContentStream, NameObject
import pdfplumber

from book_content import CHAPTERS, ANSWERS, T, C

ROOT=Path(__file__).resolve().parent
WORKSPACE=ROOT.parents[2]
SOURCE=WORKSPACE/'.codex-remote-attachments/01a13344-a365-7cf0-930f-59d4c3a9ea5c/05b6517b-f981-4fc0-af20-555a58edf41e/1-OS_merged_chapters_02-23.pdf'
BASE=WORKSPACE/'output/pdf/운영체제_핵심통합요약.pdf'
OUT=WORKSPACE/'output/pdf/운영체제_핵심요약_문제_정답해설.pdf'
PIECES=ROOT/'pieces'; PIECES.mkdir(exist_ok=True)
W,H=A4; M=46; CW=W-2*M
NAVY=colors.HexColor('#18354c'); TEAL=colors.HexColor('#087f8c')
INK=colors.HexColor('#243745'); GRAY=colors.HexColor('#5c6b78')
PALE=colors.HexColor('#eff5f7'); LINE=colors.HexColor('#d8e3e8')
pdfmetrics.registerFont(TTFont('Malgun','C:/Windows/Fonts/malgun.ttf'))
pdfmetrics.registerFont(TTFont('MalgunBold','C:/Windows/Fonts/malgunbd.ttf'))
pdfmetrics.registerFont(TTFont('Consolas','C:/Windows/Fonts/consola.ttf'))
pdfmetrics.registerFontFamily('Malgun',normal='Malgun',bold='MalgunBold',italic='Malgun',boldItalic='MalgunBold')
ST={
 'title':ParagraphStyle('title',fontName='MalgunBold',fontSize=24,leading=33,textColor=NAVY,spaceAfter=12,wordWrap='CJK'),
 'h1':ParagraphStyle('h1',fontName='MalgunBold',fontSize=20,leading=28,textColor=NAVY,spaceAfter=10,wordWrap='CJK'),
 'h2':ParagraphStyle('h2',fontName='MalgunBold',fontSize=12.4,leading=19,textColor=TEAL,spaceBefore=11,spaceAfter=6,wordWrap='CJK',keepWithNext=True),
 'q':ParagraphStyle('q',fontName='MalgunBold',fontSize=12.5,leading=19,textColor=NAVY,spaceBefore=13,spaceAfter=6,wordWrap='CJK',keepWithNext=True),
 'body':ParagraphStyle('body',fontName='Malgun',fontSize=10.5,leading=16.3,textColor=INK,spaceAfter=7,wordWrap='CJK'),
 'small':ParagraphStyle('small',fontName='Malgun',fontSize=8.4,leading=13,textColor=GRAY,spaceAfter=6,wordWrap='CJK'),
 'cell':ParagraphStyle('cell',fontName='Malgun',fontSize=8.8,leading=13.4,textColor=INK,wordWrap='CJK'),
 'th':ParagraphStyle('th',fontName='MalgunBold',fontSize=8.8,leading=13.4,textColor=colors.white,wordWrap='CJK'),
 'code':ParagraphStyle('code',fontName='Consolas',fontSize=8.4,leading=11.5,textColor=INK,leftIndent=7,spaceAfter=7),
}
def fmt(s):
    s=str(s).replace('\u2011','-').replace('\u2013','-').replace('\u2014','-')
    return re.sub(r'<(?!/?(?:b|i|super|sub|br|a)(?:\s|>|/))','&lt;',s)
def P(s,style='body'): return Paragraph(fmt(s),ST[style])
def tb(headers,rows,widths=None):
    if widths is None: widths=[1/len(headers)]*len(headers)
    data=[[P(x,'th') for x in headers]]+[[P(x,'cell') for x in row] for row in rows]
    t=Table(data,colWidths=[CW*v for v in widths],repeatRows=1,hAlign='LEFT')
    t.setStyle(TableStyle([
     ('BACKGROUND',(0,0),(-1,0),NAVY),('ROWBACKGROUNDS',(0,1),(-1,-1),[colors.white,PALE]),
     ('VALIGN',(0,0),(-1,-1),'TOP'),('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),
     ('TOPPADDING',(0,0),(-1,-1),6),('BOTTOMPADDING',(0,0),(-1,-1),6),
     ('LINEBELOW',(0,0),(-1,0),.5,NAVY),('LINEBELOW',(0,1),(-1,-1),.35,LINE)
    ]))
    return t
def box(s):
    t=Table([[P(s)]],colWidths=[CW],hAlign='LEFT')
    t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,-1),PALE),('LINEBEFORE',(0,0),(0,-1),3,TEAL),
     ('LEFTPADDING',(0,0),(-1,-1),12),('RIGHTPADDING',(0,0),(-1,-1),12),
     ('TOPPADDING',(0,0),(-1,-1),10),('BOTTOMPADDING',(0,0),(-1,-1),10)]))
    return t

class Chart(Flowable):
    def __init__(self,name):
        Flowable.__init__(self); self.width=CW; self.height=236; self.name=name
    def draw(self):
        data=json.loads((ROOT/'graphs.json').read_text(encoding='utf-8'))[self.name]
        config={
         'lottery':('CPU length per job','Completion ratio U',['Lottery mean (500 seeds)','Stride (equal tickets)'],.7,1.0,1000),
         'relocation':('Limit (bytes)','Valid ratio',['Theory L/A','Observed (1000 addresses)'],0,1,1024),
         'working_set':('Cache capacity (pages)','Hit ratio',['LRU (synthetic locality trace)'],0,1,20),
        }[self.name]
        xlabel,ylabel,labels,ymin,ymax,xmax=config
        d=Drawing(CW,236)
        lp=LinePlot();lp.x=49;lp.y=37;lp.width=CW-65;lp.height=163
        lp.data=[[(row[0],row[i+1]) for row in data] for i in range(len(labels))]
        lp.xValueAxis.valueMin=0;lp.xValueAxis.valueMax=xmax
        lp.yValueAxis.valueMin=ymin;lp.yValueAxis.valueMax=ymax
        for axis in [lp.xValueAxis,lp.yValueAxis]:
            axis.labels.fontName='Malgun';axis.labels.fontSize=8
            axis.strokeColor=GRAY
        lp.xValueAxis.tickDown=3
        lp.yValueAxis.tickLeft=3
        lp.yValueAxis.labelTextFormat='%.2f'
        if self.name=='lottery':lp.yValueAxis.valueSteps=[.7,.8,.9,1.0]
        else:lp.yValueAxis.valueSteps=[0,.25,.5,.75,1]
        for i,label in enumerate(labels):
            col=TEAL if i==0 else NAVY
            lp.lines[i].strokeColor=col;lp.lines[i].strokeWidth=1.6
            if i==1:lp.lines[i].strokeDashArray=[4,2]
            xx=49+i*218
            d.add(Line(xx,218,xx+20,218,strokeColor=col,strokeWidth=2))
            d.add(String(xx+26,215,label,fontName='Malgun',fontSize=8.2,fillColor=INK))
        d.add(lp)
        d.add(String(CW/2,8,xlabel,fontName='Malgun',fontSize=9,textAnchor='middle',fillColor=INK))
        d.add(String(49,203,ylabel,fontName='Malgun',fontSize=8.3,fillColor=INK))
        renderPDF.draw(d,self.canv,0,0)

class StateDiagram(Flowable):
    def __init__(self):Flowable.__init__(self);self.width=CW;self.height=150
    def draw(self):
        c=self.canv
        for x,y,label in [(10,85,'Ready'),(195,85,'Running'),(195,5,'Blocked')]:
            c.setFillColor(PALE);c.setStrokeColor(LINE);c.roundRect(x,y,130,45,6,fill=1,stroke=1)
            c.setFillColor(NAVY);c.setFont('MalgunBold',11);c.drawCentredString(x+65,y+17,label)
        def a(x1,y1,x2,y2):
            c.setStrokeColor(TEAL);c.setFillColor(TEAL);c.line(x1,y1,x2,y2)
            ang=math.atan2(y2-y1,x2-x1);p=c.beginPath();p.moveTo(x2,y2)
            for d in [-.45,.45]:p.lineTo(x2-7*math.cos(ang+d),y2-7*math.sin(ang+d))
            p.close();c.drawPath(p,stroke=0,fill=1)
        a(140,118,195,118);a(195,96,140,96);a(260,85,260,50)
        c.setStrokeColor(TEAL);c.line(195,28,75,28);a(75,28,75,85)
        c.setFillColor(GRAY);c.setFont('Malgun',9)
        c.drawCentredString(168,134,'스케줄');c.drawCentredString(168,79,'선점')
        c.drawString(270,64,'I/O 요청');c.drawCentredString(130,36,'I/O 완료')
        c.drawString(351,110,'Ready: CPU 대기');c.drawString(351,85,'Blocked: 사건 대기')
        c.drawString(351,60,'I/O 완료 후 Ready')

class Doc(BaseDocTemplate):
    def __init__(self,path,part,title):
        super().__init__(str(path),pagesize=A4,leftMargin=M,rightMargin=M,topMargin=62,bottomMargin=53,
         title=title,author='Codex',pageCompression=1)
        self.part=part;self.anchors=[]
        self.addPageTemplates(PageTemplate(id='body',frames=[Frame(M,53,CW,H-115,
         leftPadding=0,rightPadding=0,topPadding=0,bottomPadding=0)],onPage=self.decorate))
    def decorate(self,c,doc):
        c.saveState();c.setFillColor(TEAL);c.rect(0,H-10,W,10,fill=1,stroke=0)
        c.setFont('Malgun',8);c.setFillColor(GRAY)
        c.drawString(M,H-34,'운영체제 핵심 요약 · 문제 · 정답 해설')
        c.drawRightString(W-M,H-34,self.part)
        c.setStrokeColor(LINE);c.setLineWidth(.5);c.line(M,H-43,W-M,H-43)
        c.restoreState()
    def afterFlowable(self,f):
        if hasattr(f,'question_number'): self.anchors.append({'n':f.question_number,'page':self.page})

def write_piece(path,part,title,flowables):
    doc=Doc(path,part,title);doc.build(flowables)
    return {'path':str(path),'pages':len(PdfReader(path).pages),'anchors':doc.anchors}

def summary_piece(ch):
    id=ch['id'];s=[P(id+'장 / 핵심 요약','small'),P(ch['title'],'h1')]
    def b(t):s.append(P(t))
    def h(t):s.append(P(t,'h2'))
    def t(head,rows,widths=None):s.extend([tb(head,rows,widths),Spacer(1,7)])
    if id=='04':
        s.append(box('<b>프로세스 = 실행 중인 프로그램 + 실행 상태</b><br/>코드만 있는 파일과 달리, 주소 공간·레지스터·PC·열린 파일 등 현재 실행에 필요한 상태를 갖는다.'))
        h('세 가지 상태와 전이');s.append(StateDiagram())
        t(['상태','기다리는 것'],[['Running','현재 CPU에서 명령 실행'],['Ready','CPU 배정'],['Blocked','I/O 등 사건의 완료']],[.25,.75])
        h('운영체제가 관리하는 정보')
        b('PCB에는 PID, 상태, 저장된 레지스터·PC·스택 포인터, 주소 공간 정보, 열린 파일과 I/O 상태 등을 보관한다. 준비 작업과 대기 작업을 관리해 사건 완료 시 적절한 작업을 깨운다.')
        h('프로그램을 실행 상태로 만들기')
        b('코드·정적 데이터를 주소 공간에 배치하고 스택·힙·인자를 준비한다. 표준 입력·출력·오류의 파일 디스크립터도 설정한다. 필요한 페이지는 요구 페이징으로 실제 접근할 때까지 탑재를 미룰 수 있다.')
        b('I/O로 한 프로세스가 막힌 동안 다른 준비 작업을 실행하면 CPU 계산과 I/O를 겹칠 수 있다. I/O 완료는 먼저 Ready로 돌아가는 사건이며 즉시 실행 여부는 스케줄링 정책에 달려 있다.')
    elif id=='05':
        s.append(box('Unix의 핵심 조합은 <b>fork → 환경 설정 → exec</b>다. 부모는 필요한 시점에 wait/waitpid로 자식 상태를 회수한다.'))
        t(['API','역할','기억할 점'],[
         ['fork','자식 생성','부모: 자식 PID / 자식:0 / 실패:-1'],
         ['exec 계열','현재 프로그램 이미지 교체','성공 후 기존 코드로 복귀하지 않으며 PID 유지'],
         ['wait/waitpid','자식 상태·종료 정보 회수','성공 반환은 PID, 종료 상태는 별도 매크로로 해석']],[.2,.4,.4])
        h('셸과 입출력 연결')
        b('셸은 fork 후 자식에서 파일 디스크립터를 바꾸고 exec를 호출한다. stdout을 파일로 연결하면 리다이렉션, pipe의 쓰기·읽기 끝을 두 프로세스에 연결하면 파이프라인이 된다. dup2는 선택한 FD 번호에 연결을 복제한다.')
        h('부모와 자식의 관계')
        b('fork 후 실행 순서는 비결정적이다. 주소 공간은 논리적으로 분리되며 COW로 복사 비용을 늦출 수 있다. fork 전에 열린 FD는 동일한 열린 파일 설명을 공유하므로 오프셋도 공유한다.')
        b('wait는 여러 자식 중 회수 가능한 하나를, waitpid는 특정 PID 등 선택한 대상을 기다릴 수 있다. WNOHANG은 아직 변화가 없으면 0을 반환한다. 자식이 없는 wait는 ECHILD 오류다.')
        h('실수하기 쉬운 부분')
        b('printf의 사용자 공간 버퍼와 커널의 파일 오프셋은 다르다. 출력 순서에는 동기화가 필요하다. 파이프의 불필요한 끝을 모두 닫아야 EOF가 전달된다. 종료 코드와 wait 반환 PID도 구분한다.')
    elif id=='13':
        s.append(box('<b>주소 공간</b>은 프로세스가 보는 가상 메모리다. 물리 배치와 사용 방식을 분리해 투명성·효율성·보호와 격리를 제공한다.'))
        t(['영역','주요 내용'],[['코드·정적 데이터','명령어, 전역·정적 변수'],['스택','함수 호출 문맥, 지역 변수, 반환 주소'],['힙','동적 할당 데이터'],['매핑 영역','공유 라이브러리, 파일·익명 메모리 등']],[.3,.7])
        h('가상 주소와 물리 주소')
        b('각 프로세스의 같은 가상 주소는 서로 다른 물리 프레임을 가리킬 수 있다. 운영체제와 MMU가 주소를 변환하고 권한을 검사한다. 프로세스가 자신의 가상 주소만 알아도 프로그램을 작성할 수 있다.')
        h('단순 그림과 실제 배치')
        b('힙과 스택이 서로 반대 방향으로 자라는 그림은 설명용 관례다. 실제 프로세스는 로더·여러 라이브러리·익명 매핑 등을 포함하며 ASLR 때문에 배치도 달라질 수 있다.')
        h('관측할 때의 구분')
        t(['지표','의미'],[['가상 크기','주소 공간에 매핑된 범위'],['RSS','현재 RAM에 상주한 페이지의 용량'],['free','현재 미사용 RAM'],['available','회수 가능한 캐시 등을 고려한 추가 사용 가능 추정치']],[.28,.72])
        b('malloc으로 큰 가상 범위를 확보했어도 실제 페이지를 쓰기 전에는 RSS가 작을 수 있다. free는 시스템 전체, pmap은 선택한 프로세스의 매핑을 보는 도구다.')
    elif id=='14':
        s.append(box('<b>malloc/free는 라이브러리 함수</b>다. 할당기는 기존 힙을 재사용하며 필요하면 brk/sbrk·mmap 등으로 운영체제에 공간을 요청한다.'))
        t(['API','핵심'],[['malloc(n)','n바이트 요청; 실패 시 NULL; 내용은 자동 0 초기화되지 않음'],['calloc','할당 후 0으로 초기화'],['realloc','크기 변경; 주소 이동 가능; 실패 시 기존 블록은 유지'],['free(p)','할당 블록의 시작 포인터 또는 NULL을 전달']],[.24,.76])
        h('스택·힙과 수명')
        b('지역 포인터 변수는 스택에 있어도 가리키는 malloc 블록은 힙에 있다. 스택의 지역 변수는 함수 복귀 후 유효하지 않으며, 힙 블록은 필요가 끝나면 명시적으로 해제한다.')
        h('반드시 피할 오류')
        t(['오류','원인'],[['크기 오류','sizeof(포인터)와 대상 크기 혼동, 문자열 널 바이트 누락'],['미초기화 읽기','할당했다고 내용까지 유효해지는 것은 아님'],['범위 밖 접근','100개 배열의 인덱스 100 등'],['수명 오류','메모리 누수, use-after-free, double free'],['잘못된 free','블록 중간·스택 포인터 등을 전달']],[.3,.7])
        b('realloc 결과는 임시 포인터로 받고 성공 후 원래 포인터를 바꾼다. 용량을 두 배씩 늘리는 벡터는 분할 상환 삽입 비용을 줄인다. gdb는 실행 상태, Memcheck는 접근 오류·누수 이력 파악에 도움이 된다.')
    elif id=='15':
        s.append(box('<b>동적 재배치</b><br/>0≤VA&lt;bound이면 PA=base+VA. 여기서 bound는 합법적인 가상 구간의 크기를 뜻한다.'))
        b('MMU가 모든 메모리 참조를 검사·변환한다. base/bound 변경은 특권 연산이다. base는 물리 시작점, bound는 보호 범위를 정한다.')
        t(['검사','조건'],[['가상 구간','VA&lt;bound; 끝 주소는 bound-1'],['물리 배치','base+bound≤물리 메모리 크기'],['변환','합법적인 VA에 base를 더함']],[.28,.72])
        h('짧은 예제')
        b('base=32 KiB, bound=16 KiB, VA=1 KiB이면 PA=33 KiB다. VA=16 KiB는 bound와 같아 위반이다. 주소가 범위 안에 있어도 물리 배치 자체가 RAM 끝을 넘으면 설정이 잘못됐다.')
        h('하드웨어와 OS의 역할')
        t(['하드웨어','운영체제'],[['변환·범위 검사','프로세스 생성·종료 때 물리 공간 관리'],['사용자/커널 모드','문맥 교환 때 base/bound 저장·복원'],['특권 레지스터·예외','오류 접근 처리, 중단한 주소 공간 이동 후 base 갱신']],[.42,.58])
        h('장점과 한계')
        b('작은 하드웨어로 빠른 변환과 보호를 제공한다. 하지만 주소 공간 전체를 연속 물리 구간에 배치하면 힙-스택 사이 미사용 범위도 낭비할 수 있다. 세그멘테이션은 필요한 논리 영역별 관리로 확장한다.')
    elif id=='16':
        s.append(box('<b>세그멘테이션</b>: 코드·힙·스택 등 논리 영역마다 base, 크기, 성장 방향, 보호 권한을 둔다. 사용하지 않는 큰 빈 영역을 RAM에 배치하지 않아도 된다.'))
        t(['구분','범위 검사·변환'],[['순방향','0≤offset&lt;L, PA=base+offset'],['역방향: base가 끝의 다음 주소','-L≤offset&lt;0, PA=base+offset']],[.4,.6])
        b('가상 주소 상위 비트 등으로 세그먼트를 찾고 나머지 비트에서 영역 내 위치를 얻는다. 역방향 스택은 기준 주소에서 음수 오프셋을 사용한다. 베이스의 정의에 따라 경계식을 확인한다.')
        h('공유와 보호')
        b('읽기/쓰기/실행 권한을 세그먼트별로 검사한다. 읽기 전용 코드 세그먼트를 여러 주소 공간에 매핑하면 프로세스의 논리적 독립성을 유지하며 물리 메모리를 절약한다.')
        h('외부 단편화')
        b('가변 크기의 연속 구간을 배치하므로 빈 공간이 흩어진다. 빈 합계 24 KiB가 3개로 분리돼 있으면 20 KiB 연속 요청을 못 받을 수 있다. 압축으로 합칠 수 있지만 데이터 복사와 레지스터 변경 비용이 든다.')
        t(['방식','강점','대가'],[['base/bound 전체 공간','단순한 변환·보호','미사용 가상 범위의 물리 낭비'],['세그멘테이션','논리 영역별 배치·공유','외부 단편화, 크기 성장·빈 공간 관리']],[.29,.36,.35])
        b('고정 크기 페이지는 분산된 프레임을 사용할 수 있어 기본 배치의 외부 단편화 문제를 줄인다. 페이지 테이블과 TLB라는 새로운 비용도 필요하다.')
    else:raise ValueError(id)
    s.append(P(f'원본 PDF {ch["source"][0]}-{ch["source"][1]}쪽. 다음에 해당 장의 문제 원문과 번호별 답·해설을 수록했다.','small'))
    p=PIECES/f'summary_{id}.pdf'
    result=write_piece(p,id+'장 · 요약',ch['title'],s)
    assert result['pages']==1,(id,result['pages'])
    return result

def answers_piece(ch):
    id=ch['id'];s=[P(id+'장 / 정답 · 해설','small'),P(ch['title'],'h1')]
    if id in ['04','07','08','09','10','15','16','17','18','20','22']:
        s.append(P('시뮬레이션 수치는 교재 저자의 공식 프로그램과 명시한 조건으로 검산했다. 미지정 옵션은 해당 버전의 기본값을 사용한다.','small'))
    elif id in ['05','06','13','14','19','21']:
        s.append(P('코드는 Linux/POSIX용 예시다. 이 Windows 작업 환경에서 Linux 하드웨어 수치를 실측하지 않았다. 실측 문항은 방법·해석·예상 경향을 답으로 제시한다.','small'))
    for a in ANSWERS[id]:
        label=('측정 과제 '+str(a['n'])) if id=='06' else ('문제 '+str(a['n']))
        heading=P(label+' · '+a['title'],'q');heading.question_number=a['n'];s.append(heading)
        s.append(P('<b>답</b> '+a['answer']))
        s.append(P('<b>해설</b> '+a['explanation']))
        for e in a['extra']:
            if e['type']=='table':s.extend([tb(e['headers'],e['rows'],e.get('widths')),Spacer(1,9)])
            elif e['type']=='code':
                s.append(P(e['caption'],'small'))
                style=copy.copy(ST['code'])
                mx=max(pdfmetrics.stringWidth(line,'Consolas',8.4) for line in e['text'].splitlines())
                if mx>CW-14:style.fontSize=8.4*(CW-14)/mx;style.leading=style.fontSize*1.36
                assert style.fontSize>=7.3,(id,a['n'],style.fontSize)
                s.append(Preformatted(e['text'],style))
            elif e['type']=='figure':s.extend([Chart(e['name']),P(e['caption'],'small'),Spacer(1,7)])
    return write_piece(PIECES/f'answers_{id}.pdf',id+'장 · 정답 해설',ch['title']+' 정답 해설',s)

def references_piece():
    s=[P('부록 / 검산 기준과 출처','small'),P('답의 기준과 재현 방법','h1'),
     box('<b>문제는 첨부 PDF의 원문을 그대로 수록했다.</b><br/>정답·해설은 이 문서에서 작성한 예시 풀이이며, 교재 저자가 배포한 공식 정답집을 복제한 것이 아니다.'),
     P('시뮬레이션 검산','h2'),
     P('공식 OSTEP Homework 저장소의 고정 커밋 afb36ca8ddbf81d847d18f6bd18a87f0a18667f2를 사용했다. 최초 155개와 추가 20개 설정을 실행했다. 의도적으로 제약을 어긴 입력의 오류도 확인했다. 무작위 그래프는 명시한 시드·모형으로 생성했다.'),
     P('<a href="https://github.com/remzi-arpacidusseau/ostep-homework/tree/afb36ca8ddbf81d847d18f6bd18a87f0a18667f2" color="#087f8c">교재 저자 공식 저장소 · 검산에 사용한 버전</a>'),
     P('숫자 답을 읽을 때','h2'),
     P('단위 틱은 시뮬레이터의 모델 시간이다. 실제 CPU의 ns·ms 측정값과 구분한다. 무작위 문제는 시드뿐 아니라 기본값·프로그램 버전·난수 소비 순서에도 의존한다. 따로 예시 조건을 정한 자유형 문제에는 그 조건을 표시했다.'),
     P('실측·코드 문제','h2'),
     P('시스템 콜·문맥 교환·TLB·free/pmap/vmstat 수치는 환경에 따라 달라진다. 이 책은 실제 사용자 Linux 장비의 측정 결과를 만들어 넣지 않았다. 코드와 관측 지표, 해석, 결과를 확인할 명령을 제시했다. 오류 예시는 의도적으로 잘못된 코드를 표시했다.'),
     P('번역·표기 차이','h2'),
     tb(['위치','해설에 반영한 차이'],[
       ['04장 문제 4','4:200은 CPU 비율 표기의 오기. 4:100과 결과가 같음을 확인'],
       ['14장 문제 5','data[I00]은 data[100]'],
       ['15장 문제 3','영문 원문은 bound가 아닌 base의 최댓값을 질문'],
       ['22장 문제 4','원본 보충 256쪽에서 LRU와 RAND 비교 방향을 교정'],
       ['23장','첨부 판본에 독립 숙제 없음; Linux 보충은 요약에 포함']],[.27,.73]),
     P('참고한 1차 자료','h2'),
     P('<a href="https://pages.cs.wisc.edu/~remzi/OSTEP/vm-mechanism.pdf" color="#087f8c">OSTEP 15장 영문 원문(문제 3 대조)</a> · '
       '<a href="https://man7.org/linux/man-pages/man2/fork.2.html" color="#087f8c">fork(2)</a> · '
       '<a href="https://man7.org/linux/man-pages/man2/wait.2.html" color="#087f8c">wait(2)</a> · '
       '<a href="https://man7.org/linux/man-pages/man3/malloc.3.html" color="#087f8c">malloc/realloc(3)</a>','small'),
     P('<a href="https://man7.org/linux/man-pages/man1/free.1.html" color="#087f8c">procps-ng free(1)</a> · '
       '<a href="https://man7.org/linux/man-pages/man8/vmstat.8.html" color="#087f8c">procps-ng vmstat(8)</a> · '
       '<a href="https://valgrind.org/docs/manual/lk-manual.html" color="#087f8c">Valgrind Lackey 공식 매뉴얼</a>','small')]
    return write_piece(PIECES/'references.pdf','부록 · 출처','검산 기준과 출처',s)

base_reader=PdfReader(BASE);source_reader=PdfReader(SOURCE)
base_plumber=pdfplumber.open(BASE)
bundles=[]
for ch in CHAPTERS:
    sm=summary_piece(ch) if not ch['summary'] else None
    ans=answers_piece(ch) if ch['count'] else None
    bundles.append({'ch':ch,'new_summary':sm,'answers':ans})
    print('CHAPTER',ch['id'],'summary',sm['pages'] if sm else len(ch['summary']),'questions',len(ch['questions']),'answers',ans['pages'] if ans else 0,flush=True)
refs=references_piece()

def positions(front_count):
    p=front_count+1;rows=[]
    for b in bundles:
        ch=b['ch'];sm=b['new_summary']['pages'] if b['new_summary'] else len(ch['summary'])
        entry={'id':ch['id'],'title':ch['title'],'summary':p,'q':p+sm if ch['questions'] else None,
          'answers':p+sm+len(ch['questions']) if b['answers'] else None,'count':ch['count']}
        rows.append(entry);p+=sm+len(ch['questions'])+(b['answers']['pages'] if b['answers'] else 0)
    return rows,p+2+refs['pages']-1

def front_piece(rows,total):
    s=[P('OPERATING SYSTEMS / STUDY EDITION','small'),P('운영체제\n핵심 요약 · 문제 · 정답 해설'.replace('\n','<br/>'),'title'),
      P(f'19개 장의 요약 · 원문 번호 문제 103개 · 별도 측정 과제 2개 · 총 {total}쪽','small'),
      box('각 장을 <b>요약 → 문제 원문 → 번호별 답·해설</b> 순서로 읽는다.<br/>문제 원문을 먼저 풀고 뒤의 답과 풀이를 확인할 수 있게 구성했다.'),
      P('이번 확장본에 추가한 것','h2'),
      P('각 장의 원본 문제 페이지, 계산·시뮬레이션의 검산표, 실행 순서·주소 변환 추적, 코드 과제의 예제, 실측 과제의 관측·해석 방법을 포함했다. 기존에 한 페이지에서 함께 요약했던 04·05, 13·14, 15·16장은 분리했다.'),
      P('답의 세 가지 유형','h2'),
      tb(['유형','내용'],[['계산·시뮬레이션','입력·기본값·시드를 명시하고 수치와 추적 결과를 검산'],['코드 과제','Linux/POSIX용 예시 코드와 정상·오류 동작 해석'],['실측·자유형 과제','정해진 단일 숫자 대신 재현 방법·예상 경향·결과 판별 기준']],[.25,.75]),
      P('문제의 범위와 쪽수','h2'),
      P('첨부 PDF에 수록된 19개 장을 기준으로 했다. 02·23장에는 독립 숙제가 없어 요약만 수록했다. 13장의 영어 문제와 한국어 번역은 동일하므로 한국어 원문 한 번을 싣고 답 8개를 연결했다. 06장의 번호 없는 측정 숙제는 두 과제로 나눠 해설했다.'),
      P('원본 PDF 쪽은 첨부 파일의 첫 페이지를 1로 센 위치다. 문제 페이지에 인쇄된 구판의 장·쪽번호를 보존했으며, 이 통합본의 위치는 바깥쪽 장 표시와 새 쪽번호로 확인한다.','small'),
      P('자료: OS_merged_chapters_02-23.pdf, 280쪽 / 기존 20쪽 핵심 요약 / 교재 저자 OSTEP Homework 프로그램. 작성·검산 기준과 1차 자료 링크는 마지막 부록에 있다.','small'),
      PageBreak(),P('CONTENTS / 장별 바로 찾기','small'),P('목차','h1')]
    s.append(tb(['장','주제','요약','문제','답·해설'],[[r['id'],r['title'],r['summary'],r['q'] or '-',r['answers'] or '-'] for r in rows],[.08,.48,.13,.13,.18]))
    s.extend([Spacer(1,8),P('장 번호는 첨부 북마크를 기준으로 통일했다. 03·11·12장은 첨부 파일에 없다. 문제 원문과 해설에 책갈피를 넣어 바로 이동할 수 있다.','small'),
      P('부록: 핵심 공식과 검산 → 마지막 복습 → 답의 기준과 출처. 기존 요약의 공식·비교표도 보존했다.','small')])
    return write_piece(PIECES/'front.pdf','표지 · 목차','운영체제 핵심 요약 문제 정답 해설',s)

rows,total=positions(2);front=front_piece(rows,total)
if front['pages']!=2:
    rows,total=positions(front['pages']);front=front_piece(rows,total)

def remove_text_in_regions(page,regions):
    stream=ContentStream(page['/Contents'],page.pdf)
    identity=(1,0,0,1,0,0);ctm=identity;stack=[];output=[];block=None;points=[]
    def mul(l,r):
        a,b,c,d,e,f=l;g,h,i,j,k,z=r
        return (a*g+c*h,b*g+d*h,a*i+c*j,b*i+d*j,a*k+c*z+e,b*k+d*z+f)
    for operands,op in stream.operations:
        if op==b'q':stack.append(ctm)
        elif op==b'Q':ctm=stack.pop() if stack else identity
        elif op==b'cm':ctm=mul(ctm,tuple(map(float,operands)))
        if op==b'BT':block=[];points=[]
        if block is not None:
            block.append((operands,op))
            if op==b'Tm':
                xx,yy=float(operands[4]),float(operands[5])
                a,b,c,d,e,f=ctm;points.append((a*xx+c*yy+e,b*xx+d*yy+f))
            if op==b'ET':
                hidden=any(rx-1<=x<=rx+rw+1 and ry-2<=y<=ry+rh+2 for x,y in points for rx,ry,rw,rh in regions)
                if not hidden:output.extend(block)
                block=None
        else:output.append((operands,op))
    stream.operations=output
    page[NameObject('/Contents')]=stream

def overlay_old(page_index,ch=None):
    blank=PageObject.create_blank_page(width=W,height=H)
    blank.merge_page(base_reader.pages[page_index-1])
    regions=[(0,0,W,44)]
    buf=BytesIO();c=Canvas(buf,pagesize=A4)
    c.setFillColor(colors.white);c.rect(0,0,W,44,fill=1,stroke=0)
    if ch:
        regions.append((M-2,H-80,CW+4,20))
        c.rect(M-2,H-80,CW+4,20,fill=1,stroke=0)
        c.setFillColor(GRAY);c.setFont('Malgun',8.4);c.drawString(M,H-75,ch['id']+'장 / 핵심 요약')
    replacements={
      1:('구성:','02장에는 독립 숙제·연습문제가 없다. 이 요약 뒤에는 04장 프로세스 요약과 해당 문제·정답 해설이 이어진다.'),
      16:('저장 장치가 RAM보다','저장 장치의 비용은 낮은 미스율에서도 큰 영향을 준다. 원본 보충의 개정 AMAT 식과 검산은 이 통합본 부록 「핵심 공식과 검산」을 참조한다.'),
      20:('학습 순서:','각 장의 요약으로 연결 관계를 잡고, 문제 원문을 푼 뒤 같은 장의 번호별 해설로 확인한다. 이 통합본의 목차와 PDF 북마크를 이용해 이동할 수 있다.')}
    if page_index in replacements:
        needle,text=replacements[page_index]
        chars=base_plumber.pages[page_index-1].chars
        joined=''.join(z['text'] for z in chars)
        loc=joined.find(needle)
        assert loc>=0,(page_index,needle)
        arr=chars[loc:]
        top=min(z['top'] for z in arr)-2
        bottom=max(z['bottom'] for z in arr)+3
        regions.append((M-2,H-bottom,CW+4,bottom-top))
        c.setFillColor(colors.white);c.rect(M-2,H-bottom,CW+4,bottom-top,fill=1,stroke=0)
        p=P(text,'small');pw,ph=p.wrap(CW,H)
        p.drawOn(c,M,H-top-ph)
    if page_index==1:
        arr=[z for z in base_plumber.pages[0].chars if z['size']>=22]
        top=min(z['top'] for z in arr)-3;bottom=max(z['bottom'] for z in arr)+3
        regions.append((M-2,H-bottom,CW+4,bottom-top))
        c.setFillColor(colors.white);c.rect(M-2,H-bottom,CW+4,bottom-top,fill=1,stroke=0)
        p=P('운영체제 개요','h1');_,ph=p.wrap(CW,H);p.drawOn(c,M,H-top-ph)
    c.save();remove_text_in_regions(blank,regions)
    blank.merge_page(PdfReader(buf).pages[0]);return blank

def source_page(n,ch):
    page=PageObject.create_blank_page(width=W,height=H)
    original=copy.copy(source_reader.pages[n-1])
    vals=list(map(float,original.trimbox));left,right=sorted([vals[0],vals[2]]);bottom,top=sorted([vals[1],vals[3]])
    original.cropbox=RectangleObject([left,bottom,right,top])
    availw=W-2*32;availh=H-104
    scale=min(availw/(right-left),availh/(top-bottom))
    dx=(W-(right-left)*scale)/2;dy=52+(availh-(top-bottom)*scale)/2
    transform=Transformation().translate(-left,-bottom).scale(scale).translate(dx,dy)
    page.merge_transformed_page(original,transform)
    buf=BytesIO();c=Canvas(buf,pagesize=A4)
    c.setFillColor(TEAL);c.rect(0,H-10,W,10,fill=1,stroke=0)
    c.setFont('MalgunBold',9);c.setFillColor(NAVY)
    c.drawString(M,H-33,ch['id']+'장 · 문제 원문')
    c.setFont('Malgun',8);c.setFillColor(GRAY);c.drawRightString(W-M,H-33,f'첨부 원본 PDF {n}쪽')
    c.save();page.merge_page(PdfReader(buf).pages[0]);return page

writer=PdfWriter();page_meta=[];all_anchors=[];chapter_meta=[]
def add_pdf(result,kind,ch=None):
    start=len(writer.pages)+1
    rr=PdfReader(result['path'])
    for p in rr.pages:
        writer.add_page(p);page_meta.append({'kind':kind,'chapter':ch['id'] if ch else None,'source':ch['source'] if ch else []})
    for a in result.get('anchors',[]):all_anchors.append({'chapter':ch['id'],'n':a['n'],'page':start+a['page']-1})
    return start

add_pdf(front,'front')
for b in bundles:
    ch=b['ch'];entry={'id':ch['id'],'title':ch['title'],'summary_start':len(writer.pages)+1,'question_start':None,'answer_start':None}
    if b['new_summary']:add_pdf(b['new_summary'],'summary',ch)
    else:
        for i in ch['summary']:
            writer.add_page(overlay_old(i,ch));page_meta.append({'kind':'summary','chapter':ch['id'],'source':ch['source']})
    if ch['questions']:
        entry['question_start']=len(writer.pages)+1
        for n in ch['questions']:
            writer.add_page(source_page(n,ch));page_meta.append({'kind':'original_questions','chapter':ch['id'],'source':[n,n]})
    if b['answers']:entry['answer_start']=add_pdf(b['answers'],'answers',ch)
    chapter_meta.append(entry)
    parent=writer.add_outline_item(ch['id']+'장 · '+ch['title'],entry['summary_start']-1)
    writer.add_outline_item('핵심 요약',entry['summary_start']-1,parent=parent)
    if entry['question_start']:writer.add_outline_item('문제 원문',entry['question_start']-1,parent=parent)
    if entry['answer_start']:
        ans_parent=writer.add_outline_item('정답·해설',entry['answer_start']-1,parent=parent)
        for a in [a for a in all_anchors if a['chapter']==ch['id']]:
            writer.add_outline_item(('측정 과제 ' if ch['id']=='06' else '문제 ')+str(a['n']),a['page']-1,parent=ans_parent)
formula_start=len(writer.pages)+1
writer.add_page(overlay_old(19));page_meta.append({'kind':'appendix','chapter':None,'source':[]})
review_start=len(writer.pages)+1
writer.add_page(overlay_old(20));page_meta.append({'kind':'appendix','chapter':None,'source':[]})
ref_start=add_pdf(refs,'references')
writer.add_outline_item('부록 · 핵심 공식과 검산',formula_start-1)
writer.add_outline_item('부록 · 마지막 복습',review_start-1)
writer.add_outline_item('부록 · 답의 기준과 출처',ref_start-1)
assert len(writer.pages)==total,(len(writer.pages),total)
assert len(all_anchors)==105

for i,(page,meta) in enumerate(zip(writer.pages,page_meta),1):
    buf=BytesIO();c=Canvas(buf,pagesize=A4)
    c.setFillColor(colors.white);c.rect(0,0,W,43,fill=1,stroke=0)
    c.setStrokeColor(LINE);c.setLineWidth(.5);c.line(M,39,W-M,39)
    c.setFillColor(GRAY);c.setFont('Malgun',7.8)
    src=meta['source'];kind=meta['kind']
    label={'front':'표지 · 목차','summary':'핵심 요약','original_questions':'문제 원문','answers':'정답 · 해설','appendix':'복습 부록','references':'검산 기준 · 출처'}[kind]
    caption=(meta['chapter']+'장 · ' if meta['chapter'] else '')+label
    if src:caption+=f' | 원본 PDF {src[0]}' +(f'-{src[1]}' if src[0]!=src[1] else '')+'쪽'
    c.drawString(M,25,caption)
    c.setFont('MalgunBold',8);c.drawRightString(W-M,25,f'{i} / {total}')
    c.save();page.merge_page(PdfReader(buf).pages[0])
writer.add_metadata({'/Title':'운영체제 핵심 요약 · 문제 · 정답 해설','/Author':'Codex',
 '/Subject':'19개 장, 원문 문제 103개와 별도 측정 과제 2개를 장별로 결합한 풀이본'})
writer.compress_identical_objects(remove_duplicates=True,remove_unreferenced=True)
with OUT.open('wb') as f:writer.write(f)
manifest={'output':str(OUT),'pages':total,'questions':105,'numbered_questions':103,'measurement_tasks':2,
 'chapters':chapter_meta,'anchors':all_anchors,'page_meta':page_meta,'formula_page':formula_start,'review_page':review_start,'references_page':ref_start}
(ROOT/'book_manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps({'output':str(OUT),'pages':total,'questions':105,'bytes':OUT.stat().st_size},ensure_ascii=False),flush=True)
