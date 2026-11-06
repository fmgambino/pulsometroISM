const vm=require('vm'),fs=require('fs'),assert=require('assert');
const elements=new Map(), prompts=[], requests=[];
class El {
 constructor(tag='div'){this.tagName=tag;this.children=[];this.dataset={};this.classList={add(){},remove(){}};this.value='';this.textContent='';this.checked=false;}
 append(x){this.children.push(x)} prepend(x){this.children.unshift(x)}
 replaceChildren(...x){this.children=x} setAttribute(k,v){this[k]=v}
}
const el=id=>{if(!elements.has(id))elements.set(id,new El());return elements.get(id)};
const context={document:{getElementById:el,createElement:t=>new El(t),querySelectorAll:()=>[]},window:{},AbortController,URLSearchParams,console,
setTimeout:()=>0,clearTimeout:()=>{},confirm:t=>(prompts.push(t),true),alert:t=>{throw Error(t)},fetch:async(path,opts)=>(requests.push({path,opts}),{ok:true,text:async()=> 'OK'})};
let source=fs.readFileSync(require('path').join(__dirname,'../web/app.js'),'utf8');
source=source.replace(/poll\(\);\s*$/,'');vm.createContext(context);vm.runInContext(source,context);
vm.runInContext(`renderRanking([{id:22,position:1,name:'Ana',ms:100,errors:0},{id:33,position:2,name:'Luis',ms:200,errors:1}]);`,context);
assert.equal(el('rank').children.length,2);
const checkbox=el('rank').children[0].children[0].children[0];checkbox.checked=true;checkbox.onchange();
assert.equal(el('deleteSelected').disabled,false);
vm.runInContext(`renderRanking([{id:33,position:1,name:'Luis',ms:200,errors:1},{id:22,position:2,name:'Ana',ms:100,errors:0}]);`,context);
assert.equal(el('rank').children[1].children[0].children[0].checked,true);
(async()=>{
 await el('deleteSelected').onclick();assert.equal(requests[0].path,'/admin/delete');assert.equal(requests[0].opts.body.get('ids'),'[22]');
 el('selectAll').checked=true;el('selectAll').onchange();
 await el('deleteSelected').onclick();assert.equal(requests[1].opts.body.get('ids'),'[33,22]');
 vm.runInContext('busy=true;selectionState()',context);assert.equal(el('deleteSelected').disabled,true);
 let spoken=0;context.window.speechSynthesis={};context.speechSynthesis={cancel(){},getVoices(){return[]},speak(){spoken++}};context.SpeechSynthesisUtterance=function(text){this.text=text};
 vm.runInContext("speak('Hola')",context);assert.equal(spoken,0);
 el('voiceToggle').onclick();assert.equal(spoken,1);
 vm.runInContext("updateAssistant({emotion:'happy',assistantText:'Excelente',phase:'won',errors:0});updateAssistant({emotion:'happy',assistantText:'Excelente',phase:'won',errors:0})",context);assert.equal(spoken,2);
 el('voiceToggle').onclick();vm.runInContext("speak('Silencio')",context);assert.equal(spoken,2);
 console.log('OK: selección por ID tras reordenar, eliminación individual/total seleccionada, bloqueo en partida y voz por gesto sin repetir sondeo');
})().catch(e=>{console.error(e);process.exitCode=1});
