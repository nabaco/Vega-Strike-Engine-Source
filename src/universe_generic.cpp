#include <stdio.h>
#include <fcntl.h>
#include <algorithm>
#include "universe_generic.h"
#include "galaxy_xml.h"
#include "galaxy_gen.h"
#include "gfx/cockpit.h"
#include "cmd/unit_generic.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "cmd/script/mission.h"
#include "vs_path.h"

using namespace GalaxyXML;

extern StarSystem *GetLoadedStarSystem(const char * file);

using namespace std;
Cockpit * Universe::isPlayerStarship(const Unit * doNotDereference) {
  if (!doNotDereference)
    return NULL;
  for (unsigned int i=0;i<cockpit.size();i++) {
    if (doNotDereference==cockpit[i]->GetParent())
      return cockpit[i];
  }
  return NULL;
}
void Universe::SetActiveCockpit (int i) {
#ifdef VS_DEBUG
  if (i<0||i>=cockpit.size()) {
    fprintf (stderr,"ouch invalid cockpit %d",i);
  }
#endif 
  current_cockpit=i;
}
void Universe::SetActiveCockpit (Cockpit * cp) {
  for (unsigned int i=0;i<cockpit.size();i++) {
    if (cockpit[i]==cp) {
      SetActiveCockpit (i);
      return;
    }
  }
}
void Universe::SetupCockpits(vector  <string> playerNames) {
	for (unsigned int i=0;i<playerNames.size();i++) {
	  cockpit.push_back( new Cockpit ("",NULL,playerNames[i]));
	}
}
void SortStarSystems (std::vector <StarSystem *> &ss, StarSystem * drawn) {
  if ((*ss.begin())==drawn) {
    return;
  }
  vector<StarSystem*>::iterator drw = std::find (ss.begin(),ss.end(),drawn);
  if (drw!=ss.end()) {
    StarSystem * tmp=drawn;
    vector<StarSystem*>::iterator i=ss.begin();
    while (i<=drw) {
      StarSystem * t=*i;
      *i=tmp;
      tmp = t;
      i++;
    }
  }
}
void Universe::Init()
{
	// No need to load weapons on server side
	//LoadWeapons("weapon_list.xml");

    LoadFactionXML("factions.xml");	
	//this->galaxy = new GalaxyXML::Galaxy (galaxy);

	script_system=NULL;
}
Universe::Universe(int argc, char** argv, const char * galaxy)
{
	this->Init();
}
Universe::Universe()
{
	script_system=NULL;
}

Universe::~Universe()
{
  //unsigned int i;
  /*
  for (i=0;i<factions.size();i++) {
    delete factions[i];
  }
  for (i=0;i<cockpit.size();i++) {
    delete cockpit[i];
  }
  */
  factions.clear();
  cockpit.clear();
}

void Universe::LoadStarSystem(StarSystem * s) {
  star_system.push_back (s);
  SortStarSystems(star_system,s);//dont' want instadie
}
bool Universe::StillExists (StarSystem * s) {
  return std::find (star_system.begin(),star_system.end(),s)!=star_system.end();
}

void Universe::UnloadStarSystem (StarSystem * s) {
  //not sure what to do here? serialize?
}
StarSystem * Universe::Init (string systemfile, const Vector & centr,const string planetname) {
  string fullname=systemfile+".system";
  return GenerateStarSystem((char *)fullname.c_str(),"",centr);
}

extern void micro_sleep (unsigned int howmuch);
extern int getmicrosleep ();

StarSystem *Universe::getStarSystem(string name){

  vector<StarSystem*>::iterator iter;

  for(iter = star_system.begin();iter!=star_system.end();iter++){
    StarSystem *ss=*iter;
    if(ss->getName()==name){
      return ss;
    }
  }

  return NULL;
}
extern void SetStarSystemLoading (bool value);
extern void MakeStarSystem (string file, Galaxy *galaxy, string origin, int forcerandom);
extern string RemoveDotSystem (const char *input);

void Universe::Generate1( const char * file, const char * jumpback)
{
  int count=0;
  SetStarSystemLoading (true);
  while (GetCorrectStarSysPath (file).length()==0) {
    MakeStarSystem(file, galaxy,RemoveDotSystem (jumpback),count);
    count++;
  }
}

void Universe::Generate2( StarSystem * ss)
{
  static bool firsttime=true;
  LoadStarSystem (ss);

  pushActiveStarSystem(ss);
  for (int tume=0;tume<=6;++tume) {
    fprintf (stderr,"star system ex ai\n");
    fflush(stderr);
    ss->ExecuteUnitAI();
    fprintf (stderr,"star system up phys\n");
    fflush(stderr);
    ss->UpdateUnitPhysics(true);    
    fprintf (stderr,"star system done up phys\n");
    fflush(stderr);
  }
  // notify the director that a new system is loaded (gotta have at least one active star system)
  StarSystem *old_script_system=script_system;

  script_system=ss;
  fprintf (stderr,"Loading Star System %s",ss->getFileName().c_str());
  vector <std::string> adjacent = getAdjacentStarSystems(ss->getFileName());
  for (unsigned int i=0;i<adjacent.size();i++) {
    fprintf (stderr,"\n Next To: %s",adjacent[i].c_str());
    vector <std::string> adj = getAdjacentStarSystems(adjacent[i]);
    for (unsigned int j=0;j<adj.size();j++) {
      fprintf (stderr,"\n\tNext To %s",adj[j].c_str()); 
    }
  }
  static bool first=true;
  if (!first) {
    mission->DirectorStartStarSystem(ss);
  }
  first=false;
  script_system=old_script_system;
  popActiveStarSystem();
  if (active_star_system.empty()) {
    pushActiveStarSystem (ss);
  } else {
    ss->SwapOut();
    activeStarSystem()->SwapIn();
  }
  if (firsttime) {
  	firsttime=false;
  }else {
  }
  SetStarSystemLoading (false);
}

StarSystem * Universe::GenerateStarSystem (const char * file, const char * jumpback, Vector center) {
  StarSystem *tmpcache;
  if ((tmpcache =GetLoadedStarSystem(file))) {
    return tmpcache;
  }
  this->Generate1( file, jumpback);
  StarSystem * ss = new StarSystem (file,center);
  this->Generate2( ss);
  return ss;
}
