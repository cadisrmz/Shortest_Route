#include "Railway.h"

void showRail(Rail *&r, int i)
{
	std::string j = "\t(";
	
	if (i <= 9) {
		j += '0';
	}
	
	std::cout
		<< j          <<  i
		<< " - "      << &r
		<< ") "       << r->depart()->name()
		<< " -> "     << r->arrival()->name()
		<< " ["       << r->train()
		<< "]\t= "    << r->duration()
		<< "min for " << r->price()
		<< " start "  << r->start()->h()
		<< ':'        << r->start()->min()
		<< std::endl;
}

/**
 *	Reads data from a formatted input fil and creates the hierarchy to represent
 *	a graph in which :
 *		- Stations are nodes ;
 *		- Rails are edges.
 *	When creating nodes, we maintain an adjacency list as well.
 *	A Rail links two Stations, having a start time, a cost and a duration.
 *
 *	@param ifstream railway data file
 *	@param ifstream city transports data file
 */
Railway::Railway(std::ifstream &f, std::ifstream &g)
{
	// - Data holders for input stream
	
	std::string data = "";
	
	// input price and time
	int price = 0, time = 0, train = 0;
	
	// Depart and arrival time
	Time depart, arrival;
	
	// Stations and Rail holder
	Station *station = NULL, *last = NULL;
	Rail    *inbound = NULL;
	
	// Stations traversed during a single trip using rails
	std::vector<Station*> trip;
	std::vector<Rail*>    rails;
	
	// Counters
	unsigned int i = 0;
	
	// - Parse "rails" file
	
	while (!f.eof()) {
		f >> data;
		
		if (data.compare("Feuille") == 0) {
			// Skip ":"
			f >> data;
		} else if (data.compare("prixh") == 0) {
			f >> data; // Skip ":"
			f >> price;
			
			++train;
			
			i = 0;
			while (i < rails.size()) {
				_rails.push_back(rails[i]);
				
				_rails.back()->setPrice(price);
				_rails.back()->setTrain(train);
				
				++i;
			}
			
			// Clear holders
			trip.clear();
			rails.clear();
		} else {
			// data = departure
			if (data.compare("25:00") != 0) {
				depart.set(data);
				// Trips are reversed compared to data
				rails.push_back(new Rail(arrival, depart));
			}
			
			// data = arrival
			f >> data;
			
			arrival.set(data);
			
			// data = city
			f >> data;
			// data = station
			f >> data;
			
			// New node
			station = _stations.add(data);
			
			// As soon as we already have a station, it means we are
			// starting a trip.
			if (!trip.empty()) {
				last    = trip.back();
				inbound = rails.back();
				
				// Set stations for last Rail we have, i.e :
				// 	- last inserted as departure ;
				// 	- new one as destination.
				inbound->setTrip(last, station);
				
				// Outbound edge for departure
				last->addOut(data, inbound);
			}
			
			trip.push_back(station);
		}
	}
	
	std::map<std::string, Station*>::iterator it = _stations.begin();
	
	std::cout << "Gares : ";
	while (it != _stations.end()) {
		std::cout << it->second->name() << " ; ";
		it++;
	}
	std::cout << std::endl;
	
	// delete it;
	
	while (!g.eof()) {
		g >> data;
		
		if (data.compare("Ville") == 0) {
			// Skip ":"
			g >> data;
			// Skip city's name
			g >> data;
		} else {
			// Add edge between two stations belonging to a same city :
			// 	- with a time
			// 	- cost = 0
			station = _stations.get(data);
			
			g >> data;
			
			last    = _stations.get(data);
			
			// Time actually
			g >> time;
			
			// - Add trip both ways
			// Station->Last
			inbound = new Rail(station, last, time);
			
			_rails.push_back(inbound);
			station->addOut(last->name(), inbound);
			
			// Last->Station
			inbound = new Rail(last, station, time);
			
			_rails.push_back(inbound);
			last->addOut(station->name(), inbound);
			
		}
	}
	_chemins = new Chemin*[5];
	i = 0;	
	std::cout << "Rails : " << std::endl;
	while (i < _rails.size()) {
		showRail(_rails[i], i);
			
		++i;
	}
}

// - Methods


void Railway::Pluscourt_cout(Station* depart, Station* arrive, std::set<Rail*> interdit)
{
	std::map<std::string, Station*> temp = _stations.list();
	std::map<std::string, Station*>::iterator it;
	std::set<std::string> a_traite;
	std::map<std::string, Rail*> listerails;
	std::map<std::string, Rail*>::iterator iter2;
	std::map<std::string, Vertex*> dijkstra;
	std::map<std::string, Vertex*>::iterator iter;
	for(it= temp.begin(); it!= temp.end(); it++){
		dijkstra.insert ( std::pair<std::string, Vertex*>( (*it).first , new Vertex() ) );
		a_traite.insert( (*it).first );
	}
	// début algo
	double min_choix;
	std::string gare_traitee;
	bool fini = false;
	Station* en_cours;
	listerails.clear();
	while(!fini){
		min_choix = DBL_MAX;
		for(iter = dijkstra.begin(); iter!= dijkstra.end(); iter++){
			if(((*iter).second->cost() < min_choix) && (a_traite.find((*iter).first) != a_traite.end() )){
				min_choix = (*iter).second->cost();
				gare_traitee = (*iter).first;
			}
		}
		en_cours = get(gare_traitee);
		if(gare_traitee == arrive->name()){
			fini = true;
		}else{
			a_traite.erase(gare_traitee);
			//en_cours = this.get(gare_traitee);
			listerails = en_cours->rails();
			for(iter2 = listerails.begin() ; iter2 != listerails.end(); iter2++){
				if( (interdit.find((*iter2).second) == interdit.end())  && ( (*iter2).second->price() + dijkstra[gare_traitee]->cost() <  dijkstra[(*iter2).second->arrival()->name()]->cost() ) ){
					dijkstra[(*iter2).second->arrival()->name()]->set( (*iter2).second->price() + dijkstra[gare_traitee]->cost() , (*iter2).second );
				}
			}
		}
	}
	
	Chemin* solution_trouve = new Chemin();
	
	while(en_cours != depart){
		solution_trouve->add(dijkstra[en_cours->name()]);
		en_cours = dijkstra[en_cours->name()]->prec()->depart();
	}
	int i = 0;
	while(i < 5){
		if(solution_trouve->cost() < _chemins[i]->cost() ){
			for(int j = i+1; j < 5 ; j++){
				_chemins[j]= _chemins[j-1]; 
			}
			_chemins[i] = solution_trouve;
			i = 5;
		}else{
			i++;
		}
	}
}

Station* Railway::get(const std::string &name)
{
	return _stations.find(name);
}
