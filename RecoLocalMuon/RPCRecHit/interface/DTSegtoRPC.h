#ifndef  DTSEGTORPC_H
#define  DTSEGTORPC_H

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "DataFormats/RPCRecHit/interface/RPCRecHit.h"
#include "DataFormats/RPCRecHit/interface/RPCRecHitCollection.h"


class DTStationIndex{
public:
    DTStationIndex():_region(0),_wheel(0),_sector(0),_station(0){}
    DTStationIndex(int region, int wheel, int sector, int station) :
    _region(region),
    _wheel(wheel),
    _sector(sector),
    _station(station){}
    ~DTStationIndex(){}
    int region() const {return _region;}
    int wheel() const {return _wheel;}
    int sector() const {return _sector;}
    int station() const {return _station;}
    bool operator<(const DTStationIndex& dtind) const{
        if(dtind.region()!=this->region())
            return dtind.region()<this->region();
        else if(dtind.wheel()!=this->wheel())
            return dtind.wheel()<this->wheel();
        else if(dtind.sector()!=this->sector())
            return dtind.sector()<this->sector();
        else if(dtind.station()!=this->station())
            return dtind.station()<this->station();
        return false;
    }
    
private:
    int _region;
    int _wheel;
    int _sector;
    int _station;
};

class ObjectMap{
public:
    ObjectMap* getInstance(const edm::EventSetup& iSetup);
    const std::set<RPCDetId> getRolls(DTStationIndex dtstationindex) const {return rollstoreDT.find(dtstationindex)->second;}
    //protected:
    std::map<DTStationIndex,const std::set<RPCDetId> > rollstoreDT;
    ObjectMap(const edm::EventSetup& iSetup);
    ObjectMap();
    void fillObjectMapDT(const edm::EventSetup&);
};

class DTSegtoRPC {
public:
  explicit DTSegtoRPC(edm::Handle<DTRecSegment4DCollection> all4DSegments,const edm::EventSetup& iSetup, const edm::Event& iEvent, bool debug, double eyr,const ObjectMap*);
  ~DTSegtoRPC();
  RPCRecHitCollection* thePoints(){return _ThePoints;}
   
private:
  RPCRecHitCollection* _ThePoints; 
  edm::OwnVector<RPCRecHit> RPCPointVector;
  bool incldt;
  bool incldtMB4;
  double MinCosAng;
  double MaxD;
  double MaxDrb4;
  double MaxDistanceBetweenSegments;
  std::vector<uint32_t> extrapolatedRolls;
};



#endif
