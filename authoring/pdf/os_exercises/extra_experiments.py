from pathlib import Path
import subprocess, json, sys, random, re, statistics

ROOT=Path(__file__).resolve().parent
base=json.loads((ROOT/'results.json').read_text(encoding='utf-8'))
old={x['key']:x for x in base}
def run(key,path,args):
    r=subprocess.run([sys.executable,str(ROOT/'official'/path),*args],capture_output=True,text=True,timeout=15)
    if r.returncode and key != '15_3_overflow': raise RuntimeError(r.stderr+r.stdout)
    if key == '15_3_overflow': assert r.returncode != 0
    return {'key':key,'script':path,'args':' '.join(args),'stdout':r.stdout,'stderr':r.stderr,'returncode':r.returncode}

random.seed(2026)
hot=[]
for i in range(3000):
    hot.append(random.randrange(3) if random.random()<.9 else random.randrange(3,20))
(ROOT/'locality.trace').write_text('\n'.join(map(str,hot)),encoding='utf-8')
extra=[]
for p in ['FIFO','LRU','RAND','CLOCK','OPT']:
    for bits in ([1,2,4,8] if p=='CLOCK' else [2]):
        extra.append(run(f'22_4_{p}_{bits}','vm-beyondphys-policy/paging-policy.py',
          ['-f',str(ROOT/'locality.trace'),'-C','5','-p',p,'-b',str(bits),'-s','0','-c']))
for cap in [1,2,3,4,5,6,8,12,16,20]:
    extra.append(run(f'22_curve_C{cap}','vm-beyondphys-policy/paging-policy.py',
       ['-f',str(ROOT/'locality.trace'),'-C',str(cap),'-p','LRU','-s','0','-c']))
extra.append(run('15_3_maxbase','vm-mechanism/relocation.py',['-s','1','-n','10','-l','100','-b','16284','-c']))
extra.append(run('15_3_overflow','vm-mechanism/relocation.py',['-s','1','-n','10','-l','100','-b','16285','-c']))

def lottery(length,seed,q=1):
    random.seed(seed,version=1)
    remain=[length,length]; ticket=[100,100]; finished={}; t=0
    while len(finished)<2:
        w=int(random.random()*1000001)%sum(ticket)
        job=0 if w<ticket[0] else 1
        remain[job]-=q
        if remain[job]<0: remain[job]=0
        t+=q
        if remain[job]==0:
            finished[job]=t; ticket[job]=0
    return finished
for seed in [1,2,3]:
    observed={int(a):int(b) for a,b in re.findall(r'JOB (\d+) DONE at time (\d+)',old[f'09_3_s{seed}']['stdout'])}
    assert lottery(100,seed)==observed
lottery_plot=[]
for length in [10,20,50,100,200,500,1000]:
    vals=[]
    for seed in range(500):
        ts=list(lottery(length,seed).values())
        vals.append(min(ts)/max(ts))
    lottery_plot.append([length,statistics.mean(vals),1-1/(2*length)])

rng=random.Random(0)
vas=[int(rng.random()*1024) for _ in range(1000)]
relocation_plot=[[l,l/1024,sum(v<l for v in vas)/len(vas)] for l in range(0,1025,64)]
for l in [0,256,512,1024]:
    r=run(f'15_plot_L{l}','vm-mechanism/relocation.py',
       ['-s','0','-a','1k','-p','16k','-b','0','-l',str(l),'-n','1000','-c'])
    assert len(re.findall('VALID',r['stdout']))==sum(v<l for v in vas)

graphs={'lottery':lottery_plot,'relocation':relocation_plot,
        'working_set':[[cap,float(re.search(r'hitrate\s+([\d.]+)',next(r['stdout'] for r in extra if r['key']==f'22_curve_C{cap}')).group(1))/100] for cap in [1,2,3,4,5,6,8,12,16,20]]}
(ROOT/'extra_results.json').write_text(json.dumps(extra,ensure_ascii=False,indent=2),encoding='utf-8')
(ROOT/'graphs.json').write_text(json.dumps(graphs,ensure_ascii=False,indent=2),encoding='utf-8')
print('extra_cases',len(extra),'lottery_averages',lottery_plot)
for r in extra:
    if '22_4' in r['key']: print(r['key'],r['stdout'].splitlines()[-2:])
