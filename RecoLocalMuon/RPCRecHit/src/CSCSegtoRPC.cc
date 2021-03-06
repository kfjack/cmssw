#include <Geometry/RPCGeometry/interface/RPCGeometry.h>
#include <Geometry/CSCGeometry/interface/CSCGeometry.h>
#include <DataFormats/CSCRecHit/interface/CSCSegmentCollection.h>
#include <Geometry/CommonDetUnit/interface/GeomDet.h>
#include <Geometry/Records/interface/MuonGeometryRecord.h>
#include <Geometry/CommonTopologies/interface/TrapezoidalStripTopology.h>
#include <FWCore/Framework/interface/EDAnalyzer.h>
#include "FWCore/Framework/interface/ESHandle.h"
#include <Geometry/RPCGeometry/interface/RPCGeomServ.h>
#include <DataFormats/RPCRecHit/interface/RPCRecHit.h>
#include <DataFormats/RPCRecHit/interface/RPCRecHitCollection.h>
#include <RecoLocalMuon/RPCRecHit/interface/CSCSegtoRPC.h>



ObjectMapCSC::ObjectMapCSC(const edm::EventSetup& iSetup){
  edm::ESHandle<RPCGeometry> rpcGeo;
  edm::ESHandle<CSCGeometry> cscGeo;
  
  iSetup.get<MuonGeometryRecord>().get(rpcGeo);
  iSetup.get<MuonGeometryRecord>().get(cscGeo);
  
  for (TrackingGeometry::DetContainer::const_iterator it=rpcGeo->dets().begin();it<rpcGeo->dets().end();it++){
    if(dynamic_cast< const RPCChamber* >( *it ) != 0 ){
      auto ch = dynamic_cast< const RPCChamber* >( *it ); 
      std::vector< const RPCRoll*> roles = (ch->rolls());
      for(std::vector<const RPCRoll*>::const_iterator r = roles.begin();r != roles.end(); ++r){
	RPCDetId rpcId = (*r)->id();
	int region=rpcId.region();
	if(region!=0){
	  int station=rpcId.station();
          int ring=rpcId.ring();
          int cscring=ring;
          int cscstation=station;
	  RPCGeomServ rpcsrv(rpcId);
	  int rpcsegment = rpcsrv.segment();
	  int cscchamber = rpcsegment; //FIX THIS ACCORDING TO RPCGeomServ::segment()Definition
          if((station==2||station==3)&&ring==3){//Adding Ring 3 of RPC to the CSC Ring 2
            cscring = 2;
          }
	  CSCStationIndex ind(region,cscstation,cscring,cscchamber);
          std::set<RPCDetId> myrolls;
	  if (rollstoreCSC.find(ind)!=rollstoreCSC.end()) myrolls=rollstoreCSC[ind];
	  myrolls.insert(rpcId);
      rollstoreCSC.erase(ind);
      std::pair<CSCStationIndex,const std::set<RPCDetId>> CSCroolsWithInd(ind,myrolls);
      rollstoreCSC.insert(CSCroolsWithInd);
	  }
      }
    }
  }
}

void ObjectMapCSC::fillObjectMapCSC(const edm::EventSetup& iSetup){
    edm::ESHandle<RPCGeometry> rpcGeo;
    edm::ESHandle<CSCGeometry> cscGeo;
    
    iSetup.get<MuonGeometryRecord>().get(rpcGeo);
    iSetup.get<MuonGeometryRecord>().get(cscGeo);
    
    for (TrackingGeometry::DetContainer::const_iterator it=rpcGeo->dets().begin();it<rpcGeo->dets().end();it++){
        if(dynamic_cast< const RPCChamber* >( *it ) != 0 ){
            auto ch = dynamic_cast< const RPCChamber* >( *it );
            std::vector< const RPCRoll*> roles = (ch->rolls());
            for(std::vector<const RPCRoll*>::const_iterator r = roles.begin();r != roles.end(); ++r){
                RPCDetId rpcId = (*r)->id();
                int region=rpcId.region();
                if(region!=0){
                    int station=rpcId.station();
                    int ring=rpcId.ring();
                    int cscring=ring;
                    int cscstation=station;
                    RPCGeomServ rpcsrv(rpcId);
                    int rpcsegment = rpcsrv.segment();
                    int cscchamber = rpcsegment; //FIX THIS ACCORDING TO RPCGeomServ::segment()Definition
                    if((station==2||station==3)&&ring==3){//Adding Ring 3 of RPC to the CSC Ring 2
                        cscring = 2;
                    }
                    CSCStationIndex ind(region,cscstation,cscring,cscchamber);
                    std::set<RPCDetId> myrolls;
                    if (rollstoreCSC.find(ind)!=rollstoreCSC.end()) myrolls=rollstoreCSC[ind];
                    myrolls.insert(rpcId);
                    rollstoreCSC.erase(ind);
                    std::pair<CSCStationIndex,const std::set<RPCDetId>> CSCroolsWithInd(ind,myrolls);
                    rollstoreCSC.insert(CSCroolsWithInd);                }
            }
        }
    }
}
  
CSCSegtoRPC::CSCSegtoRPC(edm::Handle<CSCSegmentCollection> allCSCSegments, const edm::EventSetup& iSetup,const edm::Event& iEvent, bool debug, double eyr, const ObjectMapCSC *TheObjectCSC){
  
  edm::ESHandle<RPCGeometry> rpcGeo;
  edm::ESHandle<CSCGeometry> cscGeo;
  
  iSetup.get<MuonGeometryRecord>().get(rpcGeo);
  iSetup.get<MuonGeometryRecord>().get(cscGeo);
  
  MaxD=80.;

   LogDebug("RPCPointProducer") <<"CSC \t Number of CSC Segments in this event = "<<allCSCSegments->size()<<std::endl;

  _ThePoints = new RPCRecHitCollection();

  if(allCSCSegments->size()==0){
    LogDebug("RPCPointProducer") << "CSC 0 segments skiping event"<<std::endl;
  }else {
    std::map<CSCDetId,int> CSCSegmentsCounter;
    CSCSegmentCollection::const_iterator segment;
      
    int segmentsInThisEventInTheEndcap=0;
      
    for (segment = allCSCSegments->begin();segment!=allCSCSegments->end(); ++segment){
      CSCSegmentsCounter[segment->cscDetId()]++;
      segmentsInThisEventInTheEndcap++;
    }    
      
    LogDebug("RPCPointProducer") << "CSC \t loop over all the CSCSegments "<<std::endl;
    for (segment = allCSCSegments->begin();segment!=allCSCSegments->end(); ++segment){
      CSCDetId CSCId = segment->cscDetId();
	
      LogDebug("RPCPointProducer") << "CSC \t \t This Segment is in Chamber id: "<<CSCId<<std::endl;
      LogDebug("RPCPointProducer") << "CSC \t \t Number of segments in this CSC = "<<CSCSegmentsCounter[CSCId]<<std::endl;
      LogDebug("RPCPointProducer") << "CSC \t \t Is the only one in this CSC? is not ind the ring 1 or station 4? Are there more than 2 segments in the event?"<<std::endl;

      if(CSCSegmentsCounter[CSCId]==1 && CSCId.station()!=4 && CSCId.ring()!=1 && allCSCSegments->size()>=2){
	LogDebug("RPCPointProducer") << "CSC \t \t yes"<<std::endl;
	int cscEndCap = CSCId.endcap();
	int cscStation = CSCId.station();
	int cscRing = CSCId.ring();
	int rpcRegion = 1; if(cscEndCap==2) rpcRegion= -1;//Relacion entre las endcaps
	int rpcRing = cscRing;
	if(cscRing==4)rpcRing =1;
	int rpcStation = cscStation;
	int rpcSegment = CSCId.chamber();
	
	LocalPoint segmentPosition= segment->localPosition();
	LocalVector segmentDirection=segment->localDirection();
	float dz=segmentDirection.z();

	LogDebug("RPCPointProducer") << "CSC \t \t \t Information about the segment"
			   <<"RecHits ="<<segment->nRecHits()
			   <<"Angle ="<<acos(dz)*180/3.1415926<<std::endl;
		      
	LogDebug("RPCPointProducer") << "CSC \t \t Is a good Segment? dim = 4, 4 <= nRecHits <= 10 Incident angle int range 45 < "<<acos(dz)*180/3.1415926<<" < 135? "<<std::endl;

	if((segment->dimension()==4) && (segment->nRecHits()<=10 && segment->nRecHits()>=4)){
	  //&& acos(dz)*180/3.1415926 > 45. && acos(dz)*180/3.1415926 < 135.){ 
	  //&& segment->chi2()< ??)Add 3 segmentes in the endcaps???


	  LogDebug("RPCPointProducer") << "CSC \t \t yes"<<std::endl;
	  LogDebug("RPCPointProducer") << "CSC \t \t CSC Segment Dimension "<<segment->dimension()<<std::endl;
	    
	  float Xo=segmentPosition.x();
	  float Yo=segmentPosition.y();
	  float Zo=segmentPosition.z();
	  float dx=segmentDirection.x();
	  float dy=segmentDirection.y();
	  float dz=segmentDirection.z();


	  LogDebug("RPCPointProducer") << "Creating the CSCIndex"<<std::endl;
      CSCStationIndex theindex(rpcRegion,rpcStation,rpcRing,rpcSegment);
	  LogDebug("RPCPointProducer") <<"Getting the Rolls for the given index"<<std::endl;
	
	  const std::set<RPCDetId> rollsForThisCSC = TheObjectCSC->getRolls(theindex);
		
	   
	  LogDebug("RPCPointProducer") << "CSC \t \t Getting chamber from Geometry"<<std::endl;
	  const CSCChamber* TheChamber=cscGeo->chamber(CSCId); 
	  LogDebug("RPCPointProducer") << "CSC \t \t Getting ID from Chamber"<<std::endl;
	  const CSCDetId TheId=TheChamber->id();

	  LogDebug("RPCPointProducer") << "CSC \t \t Number of rolls for this CSC = "<<rollsForThisCSC.size()<<std::endl;

	  LogDebug("RPCPointProducer") << "CSC \t \t Printing The Id"<<TheId<<std::endl;

	  if(rpcRing!=1&&rpcStation!=4){//They don't exist!
	  
	    assert(rollsForThisCSC.size()>=1);

	   LogDebug("RPCPointProducer") << "CSC \t \t Loop over all the rolls asociated to this CSC"<<std::endl;
	    for (std::set<RPCDetId>::iterator iteraRoll = rollsForThisCSC.begin();iteraRoll != rollsForThisCSC.end(); iteraRoll++){
	      const RPCRoll* rollasociated = rpcGeo->roll(*iteraRoll);
	      RPCDetId rpcId = rollasociated->id();
		
	      LogDebug("RPCPointProducer") << "CSC \t \t \t We are in the roll getting the surface"<<rpcId<<std::endl;
	      const BoundPlane & RPCSurface = rollasociated->surface(); 

	      LogDebug("RPCPointProducer") << "CSC \t \t \t RollID: "<<rpcId<<std::endl;
		
	      LogDebug("RPCPointProducer") << "CSC \t \t \t Doing the extrapolation to this roll"<<std::endl;
	      LogDebug("RPCPointProducer") << "CSC \t \t \t CSC Segment Direction in CSCLocal "<<segmentDirection<<std::endl;
	      LogDebug("RPCPointProducer") << "CSC \t \t \t CSC Segment Point in CSCLocal "<<segmentPosition<<std::endl;  
		
	      GlobalPoint CenterPointRollGlobal = RPCSurface.toGlobal(LocalPoint(0,0,0));
	      LogDebug("RPCPointProducer") << "CSC \t \t \t Center (0,0,0) of the Roll in Global"<<CenterPointRollGlobal<<std::endl;
	      GlobalPoint CenterPointCSCGlobal = TheChamber->toGlobal(LocalPoint(0,0,0));
	      LogDebug("RPCPointProducer") << "CSC \t \t \t Center (0,0,0) of the CSC in Global"<<CenterPointCSCGlobal<<std::endl;
	      GlobalPoint segmentPositionInGlobal=TheChamber->toGlobal(segmentPosition); //new way to convert to global
	      LogDebug("RPCPointProducer") << "CSC \t \t \t Segment Position in Global"<<segmentPositionInGlobal<<std::endl;
	      LocalPoint CenterRollinCSCFrame = TheChamber->toLocal(CenterPointRollGlobal);

	      if(debug){//to check CSC RPC phi relation!
		float rpcphi=0;
		float cscphi=0;
		  
		(CenterPointRollGlobal.barePhi()<0)? 
		  rpcphi = 2*3.141592+CenterPointRollGlobal.barePhi():rpcphi=CenterPointRollGlobal.barePhi();
		  
		(CenterPointCSCGlobal.barePhi()<0)? 
		  cscphi = 2*3.1415926536+CenterPointCSCGlobal.barePhi():cscphi=CenterPointCSCGlobal.barePhi();

		float df=fabs(cscphi-rpcphi); 
		float dr=fabs(CenterPointRollGlobal.perp()-CenterPointCSCGlobal.perp());
		float diffz=CenterPointRollGlobal.z()-CenterPointCSCGlobal.z();
		float dfg=df*180./3.14159265;

		LogDebug("RPCPointProducer") << "CSC \t \t \t z of RPC="<<CenterPointRollGlobal.z()<<"z of CSC"<<CenterPointCSCGlobal.z()<<" dfg="<<dfg<<std::endl;
		  
		RPCGeomServ rpcsrv(rpcId);
		  
		if(dr>200.||fabs(dz)>55.||dfg>1.){ 
		  //if(rpcRegion==1&&dfg>1.&&dr>100.){  
		  if (debug) std::cout
		    <<"\t \t \t CSC Station= "<<CSCId.station()
		    <<" Ring= "<<CSCId.ring()
		    <<" Chamber= "<<CSCId.chamber()
		    <<" cscphi="<<cscphi*180/3.14159265
		    <<"\t RPC Station= "<<rpcId.station()
		    <<" ring= "<<rpcId.ring()
		    <<" segment =-> "<<rpcsrv.segment()
		    <<" rollphi="<<rpcphi*180/3.14159265
		    <<"\t dfg="<<dfg
		    <<" dz="<<diffz
		    <<" dr="<<dr
		    <<std::endl;
		    
		}
	      }
	      
	      float D=CenterRollinCSCFrame.z();
	  	  
	      float X=Xo+dx*D/dz;
	      float Y=Yo+dy*D/dz;
	      float Z=D;

	      const TrapezoidalStripTopology* top_=dynamic_cast<const TrapezoidalStripTopology*>(&(rollasociated->topology()));
	      LocalPoint xmin = top_->localPosition(0.);
	      LogDebug("RPCPointProducer") << "CSC \t \t \t xmin of this  Roll "<<xmin<<"cm"<<std::endl;
	      LocalPoint xmax = top_->localPosition((float)rollasociated->nstrips());
	      LogDebug("RPCPointProducer") << "CSC \t \t \t xmax of this  Roll "<<xmax<<"cm"<<std::endl;
	      float rsize = fabs( xmax.x()-xmin.x() );
	      LogDebug("RPCPointProducer") << "CSC \t \t \t Roll Size "<<rsize<<"cm"<<std::endl;
	      float stripl = top_->stripLength();
	      float stripw = top_->pitch();

	      LogDebug("RPCPointProducer") << "CSC \t \t \t Strip Lenght "<<stripl<<"cm"<<std::endl;
	      LogDebug("RPCPointProducer") << "CSC \t \t \t Strip Width "<<stripw<<"cm"<<std::endl;

	      LogDebug("RPCPointProducer") << "CSC \t \t \t X Predicted in CSCLocal= "<<X<<"cm"<<std::endl;
	      LogDebug("RPCPointProducer") << "CSC \t \t \t Y Predicted in CSCLocal= "<<Y<<"cm"<<std::endl;
	      LogDebug("RPCPointProducer") << "CSC \t \t \t Z Predicted in CSCLocal= "<<Z<<"cm"<<std::endl;
	  
	      float extrapolatedDistance = sqrt((X-Xo)*(X-Xo)+(Y-Yo)*(Y-Yo)+(Z-Zo)*(Z-Zo));

	      LogDebug("RPCPointProducer") << "CSC \t \t \t Is the distance of extrapolation less than MaxD? ="<<extrapolatedDistance<<"cm"<<" MaxD="<<MaxD<<"cm"<<std::endl;
	  
	      if(extrapolatedDistance<=MaxD){ 

		LogDebug("RPCPointProducer") << "CSC \t \t \t yes"<<std::endl;

		GlobalPoint GlobalPointExtrapolated=TheChamber->toGlobal(LocalPoint(X,Y,Z));
		LogDebug("RPCPointProducer") << "CSC \t \t \t Point ExtraPolated in Global"<<GlobalPointExtrapolated<< std::endl;

	      
		LocalPoint PointExtrapolatedRPCFrame = RPCSurface.toLocal(GlobalPointExtrapolated);

		LogDebug("RPCPointProducer") << "CSC \t \t \t Point Extrapolated in RPCLocal"<<PointExtrapolatedRPCFrame<< std::endl;
		LogDebug("RPCPointProducer") << "CSC \t \t \t Corner of the Roll = ("<<rsize*eyr<<","<<stripl*eyr<<")"<<std::endl;
		LogDebug("RPCPointProducer") << "CSC \t \t \t Info About the Point Extrapolated in X Abs ("<<fabs(PointExtrapolatedRPCFrame.x())<<","
				   <<fabs(PointExtrapolatedRPCFrame.y())<<","<<fabs(PointExtrapolatedRPCFrame.z())<<")"<<std::endl;
		LogDebug("RPCPointProducer") << "CSC \t \t \t dz="
				   <<fabs(PointExtrapolatedRPCFrame.z())<<" dx="
				   <<fabs(PointExtrapolatedRPCFrame.x())<<" dy="
				   <<fabs(PointExtrapolatedRPCFrame.y())<<std::endl;
		  
		LogDebug("RPCPointProducer") << "CSC \t \t \t Does the extrapolation go inside this roll????"<<std::endl;

		if(fabs(PointExtrapolatedRPCFrame.z()) < 1. && 
		   fabs(PointExtrapolatedRPCFrame.x()) < rsize*eyr && 
		   fabs(PointExtrapolatedRPCFrame.y()) < stripl*eyr){ 
		  LogDebug("RPCPointProducer") << "CSC \t \t \t \t yes"<<std::endl;
		  LogDebug("RPCPointProducer") << "CSC \t \t \t \t Creating the RecHit"<<std::endl;
		  RPCRecHit RPCPoint(rpcId,0,PointExtrapolatedRPCFrame);
		  LogDebug("RPCPointProducer") << "CSC \t \t \t \t Clearing the vector"<<std::endl;	
		  RPCPointVector.clear();
		  LogDebug("RPCPointProducer") << "CSC \t \t \t \t Pushing back"<<std::endl;	
		  RPCPointVector.push_back(RPCPoint); 
		  LogDebug("RPCPointProducer") << "CSC \t \t \t \t Putting the vector"<<std::endl;	
		  _ThePoints->put(rpcId,RPCPointVector.begin(),RPCPointVector.end());
		}
	      }
	    }
	  }
	}
      }
    }
  }
}

CSCSegtoRPC::~CSCSegtoRPC(){

}
