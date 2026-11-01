
(() => {
  const root=document.getElementById('polygon-cut-trace');
  const select=root.querySelector('#pct-method'),prev=root.querySelector('#pct-prev'),next=root.querySelector('#pct-next'),count=root.querySelector('#pct-count'),detail=root.querySelector('#pct-detail'),figure=root.querySelector('#pct-figure');
  const polygon=[[-4,5],[3,10],[3,6],[-1,4],[4,0],[-4,0]];
  const A=[0,55/7],B=[0,4.5],C=[0,3.2],D=[0,0];
  const upper=[A,[3,10],[3,6],B],lower=[C,[4,0],D];
  const cutLabels=['원래 도형','왼쪽 경계 밖을 잘라냄','오른쪽 경계 검사 · 추가로 잘릴 부분 없음','아래쪽 경계 검사 · 추가로 잘릴 부분 없음','위쪽 경계 검사 · 완료'];
  const traceLabels=['원래 도형','윗조각: A에서 B까지 원래 도형의 선을 따라감','윗조각: B에서 A까지 윈도우의 선을 따라 닫음','아랫조각: C에서 D까지 원래 도형의 선을 따라감','아랫조각: D에서 C까지 윈도우의 선을 따라 닫음','완료 · 서로 떨어진 두 조각'];
  let mode='cut',step=0;
  function applySaved(s){const p=s && s.privateContent;if(p && ['cut','trace'].includes(p.method)){mode=p.method;step=Math.max(0,Math.min(mode==='cut'?4:5,Number(p.step)||0));}select.value=mode;}
  applySaved(window.openai && window.openai.widgetState);
  function persist(){if(window.openai && window.openai.setWidgetState)window.openai.setWidgetState({modelContent:{method:mode==='cut'?'Sutherland–Hodgman':'Weiler–Atherton',step:step},privateContent:{method:mode,step:step}}).catch(()=>{});}
  function clipLeft(points){const out=[];for(let i=0;i<points.length;i++){const e=points[i],s=points[(i+points.length-1)%points.length],si=s[0]>=0,ei=e[0]>=0;if(si!==ei){const t=-s[0]/(e[0]-s[0]);out.push([0,s[1]+t*(e[1]-s[1])]);}if(ei)out.push(e);}return out;}
  function draw(){
    const width=Math.max(280,figure.getBoundingClientRect().width||680),height=Math.min(380,Math.max(280,width*.56));
    const unit=Math.min((width-66)/13,(height-46)/13),ox=(width-13*unit)/2,oy=(height-13*unit)/2;
    const P=p=>[ox+(p[0]+5)*unit,oy+(12-p[1])*unit];
    const pts=arr=>arr.map(p=>P(p).join(',')).join(' ');
    const path=(arr,series,active=false)=>'<polyline '+(active?'class="pct-new-trace" pathLength="1"':'')+' points="'+pts(arr)+'" fill="none" stroke="var(--viz-series-'+series+')" stroke-width="4" stroke-linejoin="round" stroke-linecap="round"/>';
    const area=arr=>'<polygon points="'+pts(arr)+'" fill="var(--viz-series-1)" fill-opacity=".19"/>';
    const labels=(mode==='cut'?cutLabels:traceLabels);
    let content='<polygon points="'+pts(polygon)+'" fill="var(--muted)" fill-opacity=".5" stroke="var(--muted-foreground)" stroke-width="1.4" opacity=".5"/>';
    const tl=P([0,11]),br=P([7,-1]);
    content+='<rect x="'+tl[0]+'" y="'+tl[1]+'" width="'+(br[0]-tl[0])+'" height="'+(br[1]-tl[1])+'" fill="none" stroke="var(--foreground)" stroke-width="1.3"/>';
    const tag=P([6.7,10.55]);content+='<text x="'+tag[0]+'" y="'+tag[1]+'" text-anchor="end">윈도우</text>';
    if(mode==='cut'){
      if(step>0)content+=area(clipLeft(polygon));
      if(step>0){const borders=[null,[[0,-1],[0,11]],[[7,-1],[7,11]],[[0,-1],[7,-1]],[[0,11],[7,11]]];content+=path(borders[step],2,true);}
      if(step===4){content+=path([...upper,A],1)+path([...lower,C],1);}
    }else{
      if(step>=2)content+=area(upper);
      if(step>=4)content+=area(lower);
      const tracks=[{p:[A,[3,10],[3,6],B],c:1},{p:[B,A],c:2},{p:[C,[4,0],D],c:1},{p:[D,C],c:2}];
      tracks.forEach((t,i)=>{if(step>i)content+=path(t.p,t.c,step===i+1);});
      if(step===5)content+=path([...upper,A],1)+path([...lower,C],1);
    }
    if(mode==='trace'){
      [['A',A],['B',B],['C',C],['D',D]].forEach(([name,p])=>{const q=P(p);content+='<circle cx="'+q[0]+'" cy="'+q[1]+'" r="4" fill="var(--foreground)"/><text x="'+(q[0]-13)+'" y="'+(q[1]+4)+'" text-anchor="end">'+name+'</text>';});
    }
    figure.innerHTML='<svg class="pct-diagram" viewBox="0 0 '+width+' '+height+'" width="'+width+'" height="'+height+'" role="img" aria-label="'+(mode==='cut'?'다각형을 윈도우의 왼쪽, 오른쪽, 아래쪽, 위쪽 경계로 차례로 검사':'교차점 A부터 윗조각을 닫고 C부터 아랫조각을 닫는 과정')+'"><title>같은 도형을 자르는 두 방법</title>'+content+'</svg>';
    prev.disabled=step===0;next.disabled=step===labels.length-1;count.textContent=(step+1)+' / '+labels.length;detail.textContent=labels[step];
    root.querySelector('#pct-legend').hidden=mode==='cut';
    requestAnimationFrame(()=>{const p=root.querySelector('.pct-new-trace');if(p)p.classList.add('pct-drawn');});
  }
  select.addEventListener('change',()=>{mode=select.value;step=0;draw();persist();});
  prev.addEventListener('click',()=>{step--;draw();persist();});
  next.addEventListener('click',()=>{step++;draw();persist();});
  window.addEventListener('openai:set_globals',e=>{const s=e.detail && e.detail.globals && e.detail.globals.widgetState;if(s){applySaved(s);draw();}});
  new ResizeObserver(draw).observe(figure);
  draw();
})();
