from pathlib import Path
from PIL import Image, ImageDraw
import json
import pdfplumber
from pypdf import PdfReader
from reportlab.pdfbase.ttfonts import TTFont

root=Path(__file__).resolve().parent
m=json.loads((root/'book_manifest.json').read_text(encoding='utf-8'))
pdf=pdfplumber.open(m['output'])
font=TTFont('audit','C:/Windows/Fonts/malgun.ttf')
bad=[];missing=[]
for i,p in enumerate(pdf.pages):
    for c in p.chars:
        if 'Malgun' in c['fontname'] or 'Consolas' in c['fontname']:
            if c['x0']<26 or c['x1']>p.width-26 or (50<c['top']<792 and c['bottom']>795):
                bad.append((i+1,c['text'],c['x0'],c['x1'],c['bottom']))
            if 'Malgun' in c['fontname'] and not c['text'].isspace() and any(not font.face.charToGlyph.get(ord(x),0) for x in c['text']):
                missing.append((i+1,c['text']))
    assert f'{i+1} / {m["pages"]}' in (p.extract_text() or '')
print('bad_bounds',bad[:5],'missing_glyphs',missing[:5])
assert not bad and not missing
assert len(m['anchors'])==105
for ch in m['chapters']:
    q=[a for a in m['anchors'] if a['chapter']==ch['id']]
    if ch['question_start']:
        assert ch['summary_start']<ch['question_start']<ch['answer_start']
        assert q and min(a['page'] for a in q)>=ch['answer_start']
    else:assert not q

files=sorted(root.glob('review-*.png'))
assert len(files)==m['pages']
for k,start in enumerate(range(0,len(files),6)):
    group=files[start:start+6];tw,th=520,750
    sheet=Image.new('RGB',(tw*3,th*2),'#dfe6ea');draw=ImageDraw.Draw(sheet)
    for j,f in enumerate(group):
        im=Image.open(f).convert('RGB');im.thumbnail((tw-10,th-27))
        x=(j%3)*tw+(tw-im.width)//2;y=(j//3)*th+24
        sheet.paste(im,(x,y))
        draw.text((j%3*tw+10,j//3*th+6),f.stem,fill='#18354c')
    sheet.save(root/f'contact_{k+1:02}.png')
rr=PdfReader(m['output'])
def count_outline(items):return sum(count_outline(x) if isinstance(x,list) else 1 for x in items)
print('pages',len(rr.pages),'outline_items',count_outline(rr.outline),'contacts',k+1,'bytes',Path(m['output']).stat().st_size)
print('all_page_checks_passed')
