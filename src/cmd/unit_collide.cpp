#include "vegastrike.h"
#include "unit.h"
#include "beam.h"
#include "gfx/mesh.h"
#include "unit_collide.h"
#include "physics.h"
#include "hashtable_3d.h"

Hashtable3d <LineCollide*, char[20],char[200]> collidetable;

bool TableLocationChanged (const Vector & Mini,const Vector & minz) { 
  return (collidetable.hash_int (Mini.i)!=collidetable.hash_int (minz.i) ||
	  collidetable.hash_int (Mini.j)!=collidetable.hash_int (minz.j) ||
	  collidetable.hash_int (Mini.k)!=collidetable.hash_int (minz.k));
}
bool TableLocationChanged (const LineCollide &lc, const Vector &minx, const Vector & maxx) {
  return TableLocationChanged (lc.Mini,minx) || TableLocationChanged (lc.Maxi,maxx);
}
void KillCollideTable (LineCollide * lc) {
  collidetable.Remove (lc, lc);
}
void AddCollideQueue (LineCollide &tmp) {
  collidetable.Put (&tmp,&tmp);
}
void Unit::SetCollisionParent (Unit * name) {
    for (int i=0;i<numsubunit;i++) {
      subunits[i]->CollideInfo.object = name;
      subunits[i]->SetCollisionParent (name);
    }
}
void Unit::UpdateCollideQueue () {
  CollideInfo.lastchecked =NULL;//reset who checked it last in case only one thing keeps crashing with it;
  Vector Puffmin (Position().i-radial_size,Position().j-radial_size,Position().k-radial_size);
  Vector Puffmax (Position().i+radial_size,Position().j+radial_size,Position().k+radial_size);
  if (CollideInfo.object == NULL||TableLocationChanged(CollideInfo,Puffmin,Puffmax)) {//assume not mutable
    if (CollideInfo.object!=NULL) {
      KillCollideTable(&CollideInfo);
    }
    CollideInfo.object = this;
    CollideInfo.Mini= Puffmin;
    CollideInfo.Maxi=Puffmax;
    AddCollideQueue (CollideInfo);
  } else {
    CollideInfo.Mini= Puffmin;
    CollideInfo.Maxi=Puffmax;
  }
}

void Unit::CollideAll() {
  unsigned int i;
  vector <LineCollide*> colQ;
  collidetable.Get (&CollideInfo,colQ);
  for (i=0;i<colQ.size();i++) {
    if (colQ[i]->lastchecked==this)
      continue;//ignore duplicates
    colQ[i]->lastchecked = this;//now we're the last checked.
    //    if (colQ[i]->object > this||)//only compare against stuff bigger than you
    if ((!CollideInfo.hhuge||(CollideInfo.hhuge&&colQ[i]->type==LineCollide::UNIT))&&((colQ[i]->object>this||(!CollideInfo.hhuge&&i<collidetable.GetHuge().size()))))//the first stuffs are in the huge array
      if (
	  Position().i+radial_size>colQ[i]->Mini.i&&
	  Position().i-radial_size<colQ[i]->Maxi.i&&
	  Position().j+radial_size>colQ[i]->Mini.j&&
	  Position().j-radial_size<colQ[i]->Maxi.j&&
	  Position().k+radial_size>colQ[i]->Mini.k&&
	  Position().k-radial_size<colQ[i]->Maxi.k) {
      //continue;
      switch (colQ[i]->type) {
      case LineCollide::UNIT://other units!!!
	((Unit*)colQ[i]->object)->Collide(this);
	break;
      case LineCollide::BEAM:
	((Beam*)colQ[i]->object)->Collide(this);
	break;
      case LineCollide::BALL:
	break;
      case LineCollide::BOLT:
	break;
      case LineCollide::PROJECTILE:
	break;
      }
    }
  }
#undef COLQ
}

bool Unit::OneWayCollide (Unit * target, Vector & normal, float &dist) {//do each of these bubbled subunits collide with the other unit?
  int i;
  if (!querySphere(target->Position(),target->rSize()))
    return false;;
  if (queryBSP(target->Position(), target->rSize(), normal,dist,false)) {

    return true;
  }
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->OneWayCollide(target,normal,dist))
      return true;
  }

  return false;
}


bool Unit::Collide (Unit * target) {
  if (target==this) 
    return false;

  //unit v unit? use point sampling?
  //now first make sure they're within bubbles of each other...
  if ((Position()-target->Position()).Magnitude()>radial_size+target->radial_size)
    return false;
  //now do some serious checks
  Vector normal(-1,-1,-1);
  float dist;
  Unit * bigger;
  Unit * smaller;
  if (radial_size<target->radial_size) {
    bigger = target;
    smaller = this;
  } else {
    bigger = this;
    smaller = target;
  }
  if (!bigger->OneWayCollide(smaller,normal,dist))
    return false;
  //UNUSED BUT GOOD  float elast = .5*(smaller->GetElasticity()+bigger->GetElasticity());
  //BAD  float speedagainst = (normal.Dot (smaller->GetVelocity()-bigger->GetVelocity()));
  //BADF  smaller->ApplyForce (normal * fabs(elast*speedagainst)/SIMULATION_ATOM);
  //BAD  bigger->ApplyForce (normal * -fabs((elast+1)*speedagainst*smaller->GetMass()/bigger->GetMass())/SIMULATION_ATOM);
  //deal damage similarly to beam damage!!  Apply some sort of repel force
  if (normal.i==-1&&normal.j==-1) {
    normal = (smaller->Position()-bigger->Position());
    if (normal.i||normal.j||normal.k)
      normal.Normalize();
  }
  //NOT USED BUT GOOD  Vector farce = normal*smaller->GetMass()*fabs(normal.Dot ((smaller->GetVelocity()-bigger->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM));
  smaller->ApplyForce (normal*.4*smaller->GetMass()*fabs(normal.Dot ((smaller->GetVelocity()-bigger->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
  bigger->ApplyForce (normal*.4*(smaller->GetMass()*smaller->GetMass()/bigger->GetMass())*-fabs(normal.Dot ((smaller->GetVelocity()-bigger->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
  //each mesh with each mesh? naw that should be in one way collide
  return true;
}

void Beam::CollideHuge (const LineCollide & lc) {
  vector <LineCollide *> tmp = collidetable.GetHuge();
  for (unsigned int i=0;i<tmp.size();i++) {
    if (tmp[i]->type==LineCollide::UNIT) {
      if (lc.Mini.i< tmp[i]->Maxi.i&&
	  lc.Mini.j< tmp[i]->Maxi.j&&
	  lc.Mini.k< tmp[i]->Maxi.k&&
	  lc.Maxi.i> tmp[i]->Mini.i&&
	  lc.Maxi.j> tmp[i]->Mini.j&&
	  lc.Maxi.k> tmp[i]->Mini.k) {
	this->Collide ((Unit*)tmp[i]->object);
      }
    }
  }

}

bool Beam::Collide (Unit * target) {
  float distance;
  Vector normal;//apply shields
  Vector end (center+direction*curlength);
  if (target==owner) 
    return false;
  


  if ((distance = target->queryBSP(center,end,normal))) { 

    curlength = distance;
    impact|=IMPACT;
    
    GFXColor coltmp (Col);
    coltmp.r+=.5;
    coltmp.g+=.5;
    coltmp.b+=.5;
    if (coltmp.r>1)coltmp.r=1;
    if (coltmp.g>1)coltmp.g=1;
    if (coltmp.b>1)coltmp.b=1;
    float tmp=(curlength/range); 
    target->ApplyDamage (center+direction*curlength,normal,(damagerate*SIMULATION_ATOM*curthick/thickness)*((1-tmp)+tmp*rangepenalty),coltmp);
    return true;
  }
  return false;
}


