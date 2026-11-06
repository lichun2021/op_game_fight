"""Build the original project's full engine and explicit attribute regressions.

Configuration/recording paths are supplied to the resulting tools at runtime.
No service, listener, database, deployment or configuration export is started.
"""
from pathlib import Path
import argparse,concurrent.futures,hashlib,json,os,subprocess,time

HERE=Path(__file__).resolve().parent; PROJECT=HERE.parents[1]
parser=argparse.ArgumentParser()
parser.add_argument('--build-dir',required=True)
parser.add_argument('--cxx',default='c++')
parser.add_argument('--cc',default='cc')
parser.add_argument('--jobs',type=int,default=4)
args=parser.parse_args()
BUILD=Path(args.build_dir).resolve();BUILD.mkdir(parents=True,exist_ok=True)
OBJECTS=BUILD/'objects';LOGS=BUILD/'logs';BIN=BUILD/'bin'
for p in [OBJECTS,LOGS,BIN]:p.mkdir(exist_ok=True)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
includes=['-I'+str(PROJECT),'-I'+str(PROJECT/'test'),'-I'+str(PROJECT/'lua/include'),'-I'+str(PROJECT/'lua/lua'),'-I'+str(PROJECT/'lua/luacjson'),'-I'+str(PROJECT/'lua/luasocket')]
cpp=[]
for pattern in ['*.cpp','AI/action_hero/*.cpp','AI/projectile/*.cpp','AI/scriptbind/*.cpp','jsonx/*.cpp','AI/*.cpp','verify/*.cpp']:
 for p in sorted(PROJECT.glob(pattern)):
  if p.name not in ['rankserver.cpp','server.cpp','rankhandle.cpp','rankserialization.cpp','testverify.cpp','op38_local_main.cpp']:cpp.append(p)
c=[p for p in (PROJECT/'lua/lua').glob('*.c') if p.name not in ['lua.c','luac.c']]
c += [PROJECT/'lua/luacjson'/f for f in ['lua_cjson.c','strbuf.c','fpconv.c']]
c += [PROJECT/'lua/luasocket'/f for f in ['auxiliar.c','buffer.c','except.c','inet.c','io.c','luasocket.c','options.c','select.c','tcp.c','timeout.c','udp.c','usocket.c']]
assert len(cpp)==72 and len(c)==48,(len(cpp),len(c))
headers=[dict(path=str(p),sha256=sha(p)) for p in sorted(PROJECT.rglob('*')) if p.is_file() and p.suffix in ['.h','.hpp','.inc','.inl'] and 'build' not in p.relative_to(PROJECT).parts and '.git' not in p.parts]

def compile_source(p):
 rel=p.relative_to(PROJECT).as_posix();obj=OBJECTS/(rel.replace('/','_')+'.o')
 flags=[args.cxx,'-std=c++11','-ffp-contract=off'] if p.suffix=='.cpp' else [args.cc,'-std=gnu99']
 argv=[*flags,'-O1','-g','-Wno-deprecated-declarations',*includes,'-c',str(p),'-o',str(obj)]
 log=LOGS/('compile_'+rel.replace('/','_')+'.log');start=time.monotonic()
 with log.open('w') as stream:r=subprocess.run(argv,cwd=PROJECT,stdout=stream,stderr=subprocess.STDOUT)
 row=dict(source=str(p),sourceSHA256=sha(p),object=str(obj),objectSHA256=sha(obj) if obj.is_file() else None,argv=argv,cwd=str(PROJECT),exitCode=r.returncode,log=str(log),logSHA256=sha(log),seconds=time.monotonic()-start)
 if r.returncode:print(json.dumps(row),flush=True)
 return row

with concurrent.futures.ThreadPoolExecutor(max_workers=max(1,args.jobs)) as pool:objects=list(pool.map(compile_source,cpp+c))
report=dict(status='compiled' if all(r['exitCode']==0 for r in objects) else 'failed',sourceProject=str(PROJECT),CPP=72,C=48,objects=objects,headerInputs=headers,serviceStarted=False,configurationWritten=False,GitIndexWritten=False)
(BUILD/'build.json').write_text(json.dumps(report,indent=2)+'\n')
assert report['status']=='compiled','Full source compile failed; see build.json'

lua=BIN/'lua'
if lua.is_symlink():assert lua.resolve()==PROJECT/'lua'
else:
 assert not lua.exists();lua.symlink_to(PROJECT/'lua',target_is_directory=True)
links=[]
for name in ['full_state','wire_frame','mapped_wire','native_consumers','schema_decoder','critical_consumers']:
 entry=HERE/(name+'.cpp');binary=BIN/name
 argv=[args.cxx,'-std=c++11','-O1','-g','-ffp-contract=off',*includes,str(entry),*[row['object'] for row in objects],'-o',str(binary)]
 log=LOGS/('link_'+name+'.log')
 with log.open('w') as stream:r=subprocess.run(argv,cwd=PROJECT,stdout=stream,stderr=subprocess.STDOUT)
 row=dict(source=str(entry),sourceSHA256=sha(entry),binary=str(binary),binarySHA256=sha(binary) if binary.is_file() else None,argv=argv,exitCode=r.returncode,log=str(log),logSHA256=sha(log));links.append(row)
 assert r.returncode==0,log
assert all(sha(Path(row['path']))==row['sha256'] for row in headers)
assert all(sha(Path(row['source']))==row['sourceSHA256'] for row in objects)
report.update(status='passed_full_original_project_120_source_build_and_6_regression_links',links=links,LuaResourceDirectory=str(PROJECT/'lua'))
(BUILD/'build.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(dict(status=report['status'],CPP=72,C=48,links=len(links),report=str(BUILD/'build.json'))))
