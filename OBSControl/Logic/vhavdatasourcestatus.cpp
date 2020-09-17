#include "vhavdatasourcestatus.h"
#include <windows.h>
#include <QDebug>
#include "DebugTrace.h"
typedef struct VHAVSTATIC_STATUS_st{
    int index;
    bool monitor;
    bool window;
    bool area;
    bool camera;
    bool media;
    bool mike;
    bool speaker;
    int opera[VHAV_ACTION_END*2];
}VHAVSTATIC_STATUS;
static VHAVSTATIC_STATUS G_static_status[]={
    0,false,false,false,false,false,true,false, {65,-1,33,-1,17,-1,8,-1,6,-1,-1,2,1,-1},
    1,false,false,false,false,false,true,true,  {65,-1,33,-1,17,-1,9,-1,7,-1,-1,3,-1,0},
    2,false,false,false,false,false,false,false,{67,-1,35,-1,19,-1,10,-1,6,-1,0,-1,3,-1},
    3,false,false,false,false,false,false,true, {67,-1,35,-1,19,-1,11,-1,7,-1,1,-1,-1,2},
    4,false,false,false,false,true,true,false,  {65,-1,37,-1,21,-1,12,-1,-1,0,-1,6,5,-1},
    5,false,false,false,false,true,true,true,   {65,-1,37,-1,21,-1,13,-1,-1,1,-1,7,-1,4},
    6,false,false,false,false,true,false,false, {65,-1,39,-1,23,-1,14,-1,-1,0,4,-1,7,-1},
    7,false,false,false,false,true,false,true,  {65,-1,39,-1,23,-1,15,-1,-1,1,5,-1,-1,6},
    8,false,false,false,true,false,true,false,  {69,-1,41,-1,25,-1,-1,0,14,-1,-1,10,9,-1},
    9,false,false,false,true,false,true,true,   {69,-1,41,-1,25,-1,-1,1,15,-1,-1,11,-1,8},
    10,false,false,false,true,false,false,false,{71,-1,43,-1,27,-1,-1,2,14,-1,8,-1,11,-1},
    11,false,false,false,true,false,false,true, {71,-1,43,-1,27,-1,-1,3,15,-1,9,-1,-1,10},
    12,false,false,false,true,true,true,false,  {69,-1,45,-1,29,-1,-1,4,-1,8,-1,14,13,-1},
    13,false,false,false,true,true,true,true,   {69,-1,45,-1,29,-1,-1,5,-1,9,-1,15,-1,12},
    14,false,false,false,true,true,false,false, {69,-1,47,-1,31,-1,-1,6,-1,8,12,-1,15,-1},
    15,false,false,false,true,true,false,true,  {69,-1,47,-1,31,-1,-1,7,-1,9,13,-1,-1,14},
    16,false,false,true,false,false,true,false, {65,-1,49,-1,-1,0,24,-1,22,-1,-1,18,17,-1},
    17,false,false,true,false,false,true,true,  {65,-1,49,-1,-1,0,25,-1,23,-1,-1,19,-1,16},
    18,false,false,true,false,false,false,false,{67,-1,51,-1,-1,2,26,-1,22,-1,16,-1,19,-1},
    19,false,false,true,false,false,false,true, {67,-1,51,-1,-1,2,27,-1,23,-1,17,-1,-1,18},
    20,false,false,true,false,true,true,false,  {65,-1,53,-1,-1,4,28,-1,-1,16,-1,22,21,-1},
    21,false,false,true,false,true,true,true,   {65,-1,53,-1,-1,4,29,-1,-1,17,-1,23,-1,20},
    22,false,false,true,false,true,false,false, {65,-1,55,-1,-1,6,30,-1,-1,16,20,-1,23,-1},
    23,false,false,true,false,true,false,true,  {65,-1,55,-1,-1,6,31,-1,-1,17,21,-1,-1,22},
    24,false,false,true,true,false,true,false,  {69,-1,57,-1,-1,8,-1,16,30,-1,-1,26,25,-1},
    25,false,false,true,true,false,true,true,   {69,-1,57,-1,-1,8,-1,17,31,-1,-1,27,-1,24},
    26,false,false,true,true,false,false,false, {71,-1,59,-1,-1,10,-1,18,30,-1,24,-1,27,-1},
    27,false,false,true,true,false,false,true,  {71,-1,59,-1,-1,10,-1,19,31,-1,25,-1,-1,26},
    28,false,false,true,true,true,true,false,   {69,-1,61,-1,-1,12,-1,20,-1,24,-1,30,29,-1},
    29,false,false,true,true,true,true,true,    {69,-1,61,-1,-1,12,-1,21,-1,25,-1,31,-1,28},
    30,false,false,true,true,true,false,false,  {69,-1,63,-1,-1,14,-1,22,-1,24,28,-1,31,-1},
    31,false,false,true,true,true,false,true,   {69,-1,63,-1,-1,14,-1,23,-1,25,29,-1,-1,30},
    32,false,true,false,false,false,true,false, {65,-1,-1,0,49,-1,40,-1,38,-1,-1,34,33,-1},
    33,false,true,false,false,false,true,true,  {65,-1,-1,0,49,-1,41,-1,39,-1,-1,35,-1,32},
    34,false,true,false,false,false,false,false,{67,-1,-1,2,51,-1,42,-1,38,-1,32,-1,35,-1},
    35,false,true,false,false,false,false,true, {67,-1,-1,2,51,-1,43,-1,39,-1,33,-1,-1,34},
    36,false,true,false,false,true,true,false,  {65,-1,-1,4,53,-1,44,-1,-1,32,-1,38,37,-1},
    37,false,true,false,false,true,true,true,   {65,-1,-1,4,53,-1,45,-1,-1,33,-1,39,-1,36},
    38,false,true,false,false,true,false,false, {65,-1,-1,6,55,-1,46,-1,-1,32,36,-1,39,-1},
    39,false,true,false,false,true,false,true,  {65,-1,-1,6,55,-1,47,-1,-1,33,37,-1,-1,38},
    40,false,true,false,true,false,true,false,  {69,-1,-1,8,57,-1,-1,32,46,-1,-1,42,41,-1},
    41,false,true,false,true,false,true,true,   {69,-1,-1,8,57,-1,-1,33,47,-1,-1,43,-1,40},
    42,false,true,false,true,false,false,false, {71,-1,-1,10,59,-1,-1,34,46,-1,40,-1,43,-1},
    43,false,true,false,true,false,false,true,  {71,-1,-1,10,59,-1,-1,35,47,-1,41,-1,-1,42},
    44,false,true,false,true,true,true,false,   {69,-1,-1,12,61,-1,-1,36,-1,40,-1,46,45,-1},
    45,false,true,false,true,true,true,true,    {69,-1,-1,12,61,-1,-1,37,-1,41,-1,47,-1,44},
    46,false,true,false,true,true,false,false,  {69,-1,-1,14,63,-1,-1,38,-1,40,44,-1,47,-1},
    47,false,true,false,true,true,false,true,   {69,-1,-1,14,63,-1,-1,39,-1,41,45,-1,-1,46},
    48,false,true,true,false,false,true,false,  {65,-1,-1,16,-1,32,56,-1,54,-1,-1,50,49,-1},
    49,false,true,true,false,false,true,true,   {65,-1,-1,17,-1,33,57,-1,55,-1,-1,51,-1,48},
    50,false,true,true,false,false,false,false, {67,-1,-1,18,-1,34,58,-1,54,-1,48,-1,51,-1},
    51,false,true,true,false,false,false,true,  {67,-1,-1,19,-1,35,59,-1,55,-1,49,-1,-1,50},
    52,false,true,true,false,true,true,false,   {65,-1,-1,20,-1,36,60,-1,-1,48,-1,54,53,-1},
    53,false,true,true,false,true,true,true,    {65,-1,-1,21,-1,37,61,-1,-1,49,-1,55,-1,52},
    54,false,true,true,false,true,false,false,  {65,-1,-1,22,-1,38,62,-1,-1,48,52,-1,55,-1},
    55,false,true,true,false,true,false,true,   {65,-1,-1,23,-1,39,63,-1,-1,49,53,-1,-1,54},
    56,false,true,true,true,false,true,false,   {69,-1,-1,24,-1,40,-1,48,62,-1,-1,58,57,-1},
    57,false,true,true,true,false,true,true,    {69,-1,-1,25,-1,41,-1,49,63,-1,-1,59,-1,56},
    58,false,true,true,true,false,false,false,  {71,-1,-1,26,-1,42,-1,50,62,-1,56,-1,59,-1},
    59,false,true,true,true,false,false,true,   {71,-1,-1,27,-1,43,-1,51,63,-1,57,-1,-1,58},
    60,false,true,true,true,true,true,false,    {69,-1,-1,28,-1,44,-1,52,-1,56,-1,62,61,-1},
    61,false,true,true,true,true,true,true,     {69,-1,-1,29,-1,45,-1,53,-1,57,-1,63,-1,60},
    62,false,true,true,true,true,false,false,   {69,-1,-1,30,-1,46,-1,54,-1,56,60,-1,63,-1},
    63,false,true,true,true,true,false,true,    {69,-1,-1,31,-1,47,-1,55,-1,57,61,-1,-1,62},
    64,true,false,false,false,false,true,false, {-1,0,-1,-1,-1,-1,68,-1,-1,-1,-1,66,65,-1},
    65,true,false,false,false,false,true,true,  {-1,0,-1,-1,-1,-1,69,-1,-1,-1,-1,67,-1,64},
    66,true,false,false,false,false,false,false,{-1,2,-1,-1,-1,-1,70,-1,-1,-1,64,-1,67,-1},
    67,true,false,false,false,false,false,true, {-1,2,-1,-1,-1,-1,71,-1,-1,-1,65,-1,-1,66},
    68,true,false,false,true,false,true,false,  {-1,8,-1,-1,-1,-1,-1,64,-1,-1,-1,70,69,-1},
    69,true,false,false,true,false,true,true,   {-1,8,-1,-1,-1,-1,-1,65,-1,-1,-1,71,-1,68},
    70,true,false,false,true,false,false,false, {-1,10,-1,-1,-1,-1,-1,66,-1,-1,68,-1,71,-1},
    71,true,false,false,true,false,false,true,  {-1,10,-1,-1,-1,-1,-1,67,-1,-1,69,-1,-1,70}
};

static int G_index=0;
bool VHAVDataSourceOperator(const VHAV_ACTION_ENUM &action,const VHAV_STATUS_ENUM &status,VHDATA_STATUS & statusList){
    if(G_index<0||G_index>sizeof(G_static_status)/sizeof(VHAVSTATIC_STATUS))
    {
        G_index=0;
    }

    VHAVSTATIC_STATUS currentStatus=G_static_status[G_index];
    int nextStatusIndex=currentStatus.opera[action*2+status];
    if(nextStatusIndex<0){
        return false;
    }

    char dbg[512]={0};
    sprintf(dbg,"STATUS_CHANGE [%d]--->[%d]\n",G_index,nextStatusIndex);
    qDebug()<<"VHAVDataSourceOperator"<<dbg;
    TRACE6("[VHAVDataSourceOperator] %s\n",dbg);
    
    G_index=nextStatusIndex;
    currentStatus=G_static_status[G_index];

    statusList.push_back(std::pair<VHAV_ACTION_ENUM,VHAV_STATUS_ENUM>(VHAV_ACTION_MONITOR,currentStatus.monitor?VHAV_STATUS_OPEND:VHAV_STATUS_CLOSED));
    statusList.push_back(std::pair<VHAV_ACTION_ENUM,VHAV_STATUS_ENUM>(VHAV_ACTION_WINDOW,currentStatus.window?VHAV_STATUS_OPEND:VHAV_STATUS_CLOSED));
    statusList.push_back(std::pair<VHAV_ACTION_ENUM,VHAV_STATUS_ENUM>(VHAV_ACTION_AREA,currentStatus.area?VHAV_STATUS_OPEND:VHAV_STATUS_CLOSED));
    statusList.push_back(std::pair<VHAV_ACTION_ENUM,VHAV_STATUS_ENUM>(VHAV_ACTION_CAMERA,currentStatus.camera?VHAV_STATUS_OPEND:VHAV_STATUS_CLOSED));
    statusList.push_back(std::pair<VHAV_ACTION_ENUM,VHAV_STATUS_ENUM>(VHAV_ACTION_MEDIA,currentStatus.media?VHAV_STATUS_OPEND:VHAV_STATUS_CLOSED));
    statusList.push_back(std::pair<VHAV_ACTION_ENUM,VHAV_STATUS_ENUM>(VHAV_ACTION_MIKE,currentStatus.mike?VHAV_STATUS_OPEND:VHAV_STATUS_CLOSED));
    statusList.push_back(std::pair<VHAV_ACTION_ENUM,VHAV_STATUS_ENUM>(VHAV_ACTION_SPEAKER,currentStatus.speaker?VHAV_STATUS_OPEND:VHAV_STATUS_CLOSED));

    return true;
}

