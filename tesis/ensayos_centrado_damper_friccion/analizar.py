"""Read-only CSV audit; reconstructed commands are NOT board measurements."""
import csv,json,math,hashlib,statistics
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
OUT=Path(__file__).resolve().parent
G=.60
class Motion:
 def __init__(self): self.last=None; self.x=0; self.v=0; self.still=0
 def update(self,x,t):
  if self.last is None or t-self.last>250:
   self.x=x;self.v=0;self.still=0;self.last=t;return 0
  dt=t-self.last
  if dt<=0:return self.v
  raw=max(-10,min(10,(x-self.x)*1000/dt))
  self.still=self.still+dt if abs(raw)<=.02 else 0
  if raw*self.v<0:self.v=0
  self.v+=dt/(40+dt)*(raw-self.v)
  if self.still>=40:self.v=0;self.still=40
  self.last=t;self.x=x;return self.v

def quant(x):return math.trunc(x*10000)/10000

def analyze(kind):
 p=ROOT/'SimConnectBridge/bin/Win32/RealismoSC6'/f'telemetry - {kind}.csv'
 rows=list(csv.DictReader(p.open(encoding='utf-8-sig')))
 ts=[int(r['time_ms']) for r in rows];ds=[b-a for a,b in zip(ts,ts[1:])]
 active=[r for r in rows if r['armed']=='1']
 result={'source':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'rows':len(rows),'armed_rows':len(active),'elapsed_s':(ts[-1]-ts[0])/1000,'duplicate_timestamps':sum(d==0 for d in ds),'gaps_over_250ms':[d for d in ds if d>250],'paused_rows':sum(r['paused']=='1' for r in rows),'gain_total_user':G}
 for key in ['q_ratio','q_pa','ias_knots','yoke_roll','yoke_pitch']:
  vals=[float(r[key]) for r in active]
  result[key]={'min':min(vals),'max':max(vals)}
 result['eas_max_knots']=math.sqrt(2*result['q_pa']['max']/1.225)/(.514444444)
 result['q_over_reference_rows']=sum(float(r['q_ratio'])>1 for r in active)
 # Candidate stream from the bridge's >=20ms rule. CSV does not expose actual
 # writes, intervening pause heartbeats or USB receipt timestamps. Sensitivity
 # to the initial sampling phase is reported, not a confidence interval.
 phase_results=[]
 for offset in [0,5,10,15]:
  last=ts[0]+offset-20;motions=[Motion(),Motion()];peaks=[0,0];vpeaks=[0,0];n=0
  for r in rows:
   t=int(r['time_ms'])
   if t-last<20:continue
   last=t
   if r['armed']!='1':motions=[Motion(),Motion()];continue
   n+=1
   for axis,key in enumerate(['yoke_roll','yoke_pitch']):
    x=quant(float(r[key]));v=motions[axis].update(x,t);m=max(abs(v)-.02,0)
    vpeaks[axis]=max(vpeaks[axis],abs(v))
    if kind=='centrado':
     # Static response only, excludes stiffness filter transient.
     u=-(.4 if axis==0 else .8)*G*(.15+.85*float(r['q_ratio']))*x
    else:
     resistance=.12*m if kind=='damper' else .03*m/(.2+m)
     u=-math.copysign(min(resistance,.4)*G*(.5 if axis==0 else 1),v)
    peaks[axis]=max(peaks[axis],abs(u)*255)
  phase_results.append({'offset_ms':offset,'samples':n,'peak_abs_pwm_equivalent':peaks,'peak_abs_velocity_normalized_s':vpeaks})
 result['approximate_replays']=phase_results
 if kind=='centrado':
  result['static_all_rows_peak_pwm']={axis:max(abs(float(r['yoke_'+axis]))*(.15+.85*float(r['q_ratio']))*coeff*G*255 for r in active) for axis,coeff in [('roll',.4),('pitch',.8)]}
 return result

if __name__=='__main__':
 data={k:analyze(k) for k in ['centrado','damper','friccion']}
 (OUT/'resultados.json').write_text(json.dumps(data,indent=2,ensure_ascii=False),encoding='utf-8')
 for k,d in data.items():
  print(k,json.dumps({key:d[key] for key in ['rows','elapsed_s','eas_max_knots','gaps_over_250ms','approximate_replays']},ensure_ascii=False))
