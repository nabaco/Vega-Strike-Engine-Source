#ifndef __AGGRESSIVE_AI_H
#define __AGGRESSIVE_AI_H
#include "fire.h"

class Flightgroup;
namespace Orders
{
class AggressiveAI : public FireAt
{
    bool  obedient; //am I currently obedient
    bool  last_time_insys;
    char  jump_time_check;
    float last_jump_distance;
    float last_jump_time;
    float currentpriority;
    double creationtime;
protected:
    void SignalChosenTarget();
    AIEvents::ElemAttrMap *logic;
    float   logiccurtime;
    float   interruptcurtime;
    QVector nav;
    UnitContainer navDestination;
    float   lurk_on_arrival;
    bool ProcessLogicItem( const AIEvents::AIEvresult &item );
    bool ExecuteLogicItem( const AIEvents::AIEvresult &item );
    bool ProcessLogic( AIEvents::ElemAttrMap &logic, bool inter ); //returns if found anything
    std::string last_directive;
    void ReCommandWing( Flightgroup *fg );
    bool ProcessCurrentFgDirective( Flightgroup *fg );
public:
    virtual void SetParent( Unit *parent1 );
    enum types
    {
        AGGAI, MOVEMENT, FACING, UNKNOWN, DISTANCE, METERDISTANCE, THREAT, FSHIELD, LSHIELD, RSHIELD, BSHIELD, FARMOR,
        BARMOR, LARMOR, RARMOR, HULL, RANDOMIZ, FSHIELD_HEAL_RATE, BSHIELD_HEAL_RATE, LSHIELD_HEAL_RATE,
        RSHIELD_HEAL_RATE,
        FARMOR_HEAL_RATE, BARMOR_HEAL_RATE, LARMOR_HEAL_RATE, RARMOR_HEAL_RATE, HULL_HEAL_RATE, TARGET_FACES_YOU,
        TARGET_IN_FRONT_OF_YOU, TARGET_GOING_YOUR_DIRECTION
    };
    AggressiveAI( const char *file, Unit *target = NULL );
    void ExecuteNoEnemies();
    void Execute();
    virtual std::string getOrderDescription()
    {
        return "aggressive";
    }
    void   AfterburnerJumpTurnTowards( Unit *target );
    double Fshield_prev;
    double Fshield_rate_old;
    double Fshield_prev_time;
    double Bshield_prev;
    double Bshield_rate_old;
    double Bshield_prev_time;
    double Lshield_prev;
    double Lshield_rate_old;
    double Lshield_prev_time;
    double Rshield_prev;
    double Rshield_rate_old;
    double Rshield_prev_time;
    double Farmour_prev;
    double Farmour_rate_old;
    double Farmour_prev_time;
    double Barmour_prev;
    double Barmour_rate_old;
    double Barmour_prev_time;
    double Larmour_prev;
    double Larmour_rate_old;
    double Larmour_prev_time;
    double Rarmour_prev;
    double Rarmour_rate_old;
    double Rarmour_prev_time;
    double Hull_prev;
    double Hull_rate_old;
    double Hull_prev_time;
    int    personalityseed;
};
}

#endif

