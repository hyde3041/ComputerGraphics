from pathlib import Path
import json, subprocess, sys, shlex, re
from concurrent.futures import ThreadPoolExecutor

ROOT = Path(__file__).resolve().parent
OFFICIAL = ROOT / 'official'
CASES = []
def case(key, script, args):
    CASES.append({'key':key, 'script':script, 'args':args})

proc='cpu-intro/process-run.py'
for key,args in [
 ('04_1','-l 5:100,5:100 -c -p'),
 ('04_2','-l 4:100,1:0 -c -p'),
 ('04_3','-l 1:0,4:100 -c -p'),
 ('04_4','-l 1:0,4:200 -c -p -S SWITCH_ON_END'),
 ('04_5','-l 1:0,4:100 -c -p -S SWITCH_ON_IO'),
 ('04_6','-l 3:0,5:100,5:100,5:100 -S SWITCH_ON_IO -I IO_RUN_LATER -c -p'),
 ('04_7','-l 3:0,5:100,5:100,5:100 -S SWITCH_ON_IO -I IO_RUN_IMMEDIATE -c -p'),
]: case(key,proc,args)
for seed in [1,2,3]:
    for switch in ['SWITCH_ON_IO','SWITCH_ON_END']:
        for io in ['IO_RUN_IMMEDIATE','IO_RUN_LATER']:
            case(f'04_8_s{seed}_{switch}_{io}',proc,f'-s {seed} -l 3:50,3:50 -S {switch} -I {io} -c -p')

for lens in ['200,200,200','100,200,300']:
    for policy in ['FIFO','SJF','RR']:
        case(f'07_{lens}_{policy}','cpu-sched/scheduler.py',f'-l {lens} -p {policy} -q 1 -c')

mlfq='cpu-sched-mlfq/mlfq.py'
for seed in [0,1,2]:
    case(f'08_1_s{seed}',mlfq,f'-s {seed} -n 2 -j 2 -m 20 -M 0 -q 5 -c')
for key,args in [
 ('08_2_single','-l 0,200,0 -Q 10,10,10 -c'),
 ('08_2_short','-l 0,200,0:100,20,0 -Q 10,10,10 -c'),
 ('08_2_io','-l 0,200,0:100,20,1 -Q 10,10,10 -S -c'),
 ('08_2_boost','-l 0,200,0:100,50,1:100,50,1 -Q 10,10,10 -S -B 50 -c'),
 ('08_2_account','-l 0,200,0:100,50,9 -Q 10,10,10 -i 1 -c'),
 ('08_2_quanta','-l 0,200,0:0,200,0 -Q 10,20,40 -c'),
 ('08_3','-l 0,10,0:0,10,0 -n 1 -q 2 -c'),
 ('08_4_game','-l 0,1000,0:0,1000,99 -n 2 -q 100 -i 1 -S -I -c'),
 ('08_4_fair','-l 0,1000,0:0,1000,99 -n 2 -q 100 -i 1 -I -c'),
 ('08_6_back','-l 0,40,0:0,10,2 -n 1 -q 5 -i 1 -c'),
 ('08_6_front','-l 0,40,0:0,10,2 -n 1 -q 5 -i 1 -I -c'),
]: case(key,mlfq,args)

lottery='cpu-sched-lottery/lottery.py'
for seed in [1,2,3]:
    case(f'09_1_s{seed}',lottery,f'-j 3 -s {seed} -c')
    case(f'09_2_s{seed}',lottery,f'-l 10:1,10:100 -s {seed} -c')
    case(f'09_3_s{seed}',lottery,f'-l 100:100,100:100 -s {seed} -c')
    case(f'09_4_s{seed}_q10',lottery,f'-l 100:100,100:100 -s {seed} -q 10 -c')

multi='cpu-sched-multi/multi.py'
for key,args in [
 ('10_1','-n 1 -L a:30:200 -c -t -T -C'),
 ('10_2','-n 1 -L a:30:200 -M 300 -c -t -T -C'),
 ('10_4_w5','-n 1 -L a:30:200 -M 300 -w 5 -c -t -T -C'),
 ('10_4_w20','-n 1 -L a:30:200 -M 300 -w 20 -c -t -T -C'),
 ('10_5','-n 2 -L a:100:100,b:100:50,c:100:50 -c -t -T -C'),
 ('10_6','-n 2 -L a:100:100,b:100:50,c:100:50 -A a:0,b:1,c:1 -c -t -T -C'),
 ('10_6_alt','-n 2 -L a:100:100,b:100:50,c:100:50 -A a:0,b:0,c:1 -c -t -T -C'),
]: case(key,multi,args)
for cache in [50,100]:
    for cpus in [1,2,3]:
        case(f'10_7_m{cache}_n{cpus}',multi,f'-n {cpus} -L a:100:100,b:100:100,c:100:100 -M {cache} -c')
for peek in [0,1,5,30,100]:
    case(f'10_8_p{peek}',multi,f'-n 2 -L a:100:100,b:100:50,c:100:50 -p -P {peek} -c -t -T -C')
for seed in [1,2,3]:
    for cpus in [1,2,4]:
        case(f'10_9_s{seed}_n{cpus}',multi,f'-s {seed} -n {cpus} -j 3 -R 40 -W 100 -M 100 -c')

reloc='vm-mechanism/relocation.py'
for seed in [1,2,3]: case(f'15_1_s{seed}',reloc,f'-s {seed} -c')
case('15_2_original',reloc,'-s 0 -n 10 -c')
case('15_2_limit',reloc,'-s 0 -n 10 -l 930 -c')
case('15_2_fixed',reloc,'-s 0 -n 10 -b 12418 -l 803 -c')
case('15_3',reloc,'-s 1 -n 10 -l 100 -c')
case('15_4',reloc,'-s 1 -n 10 -a 4k -p 64k -b 32768 -l 4k -c')

seg='vm-segmentation/segmentation.py'
for seed in [0,1,2]:
    case(f'16_1_s{seed}',seg,f'-a 128 -p 512 -b 0 -l 20 -B 512 -L 20 -s {seed} -c')
case('16_2',seg,'-a 128 -p 512 -b 0 -l 20 -B 512 -L 20 -A 19,20,107,108,127 -c')
case('16_3',seg,'-a 16 -p 128 -A 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 --b0 0 --l0 2 --b1 128 --l1 2 -c')
case('16_4',seg,'-a 1000 -p 4096 --b0 0 --l0 450 --b1 4096 --l1 450 -n 1000 -c')
case('16_5',seg,'-a 16 -p 128 --b0 0 --l0 0 --b1 128 --l1 0 -A 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 -c')

malloc='vm-freespace/malloc.py'
for policy in ['BEST','WORST','FIRST']:
    for order in ['ADDRSORT','SIZESORT+','SIZESORT-']:
        case(f'17_short_{policy}_{order}',malloc,f'-n 10 -H 0 -p {policy} -s 0 -l {order} -c')
for coalesce in [False,True]:
    for order in ['ADDRSORT','SIZESORT+','SIZESORT-']:
        case(f'17_long_{order}_{coalesce}',malloc,f'-n 1000 -H 0 -p BEST -s 0 -l {order} -c '+('-C' if coalesce else ''))
for alloc in [0,10,50,75,100]:
    case(f'17_6_p{alloc}',malloc,f'-n 1000 -H 0 -p BEST -s 0 -P {alloc} -C -c')
case('17_7',malloc,'-S 100 -H 0 -A +10,+10,+10,+10,+10,+10,+10,+10,+10,+10,-0,-2,-4,-6,-8,+15 -c')

paging='vm-paging/paging-linear-translate.py'
for valid in [0,25,50,75,100]:
    case(f'18_2_u{valid}',paging,f'-P 1k -a 16k -p 32k -v -u {valid} -c')
for i,args in enumerate(['-P 8 -a 32 -p 1024 -v -s 1','-P 8k -a 32k -p 1m -v -s 2','-P 1m -a 256m -p 512m -v -s 3']):
    case(f'18_3_{i+1}',paging,args+' -c')
case('18_4_bigvirtual',paging,'-P 1k -a 64k -p 32k -u 25 -c')
case('18_4_badpage',paging,'-P 3k -a 16k -p 32k -c')

multilevel='vm-smalltables/paging-multilevel-translate.py'
for seed in [0,1,2]: case(f'20_2_s{seed}',multilevel,f'-s {seed} -c')

policy='vm-beyondphys-policy/paging-policy.py'
for seed in [0,1,2]:
    for alg in ['FIFO','LRU','OPT']:
        case(f'22_1_s{seed}_{alg}',policy,f'-s {seed} -n 10 -p {alg} -c')
for alg in ['FIFO','LRU','MRU','OPT']:
    case(f'22_2_{alg}',policy,'-a '+('0,1,2,3,4,5,4,5,4,5,4,5,4,5,4,5' if alg=='MRU' else '0,1,2,3,4,5,0,1,2,3,4,5,0,1,2,3,4,5')+f' -C 5 -p {alg} -c')
for seed in [0,1,2]:
    for alg in ['FIFO','LRU','RAND','CLOCK','OPT']:
        case(f'22_3_s{seed}_{alg}',policy,f'-s {seed} -n 1000 -m 10 -C 5 -p {alg} -c')

def run(c):
    r=subprocess.run([sys.executable,str(OFFICIAL/c['script']),*shlex.split(c['args'])],capture_output=True,text=True,encoding='utf-8',timeout=25)
    return {**c,'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr}

with ThreadPoolExecutor(max_workers=4) as pool:
    results=list(pool.map(run,CASES))
(ROOT/'results.json').write_text(json.dumps(results,ensure_ascii=False,indent=2),encoding='utf-8')
for r in results:
    if r['returncode'] or 'Error:' in r['stdout']:
        print('CHECK',r['key'],r['returncode'],r['stdout'][-300:],r['stderr'][:300])
print('cases',len(results),'saved',str(ROOT/'results.json'))
