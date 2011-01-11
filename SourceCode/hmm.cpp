// Hidden Markov model class
// Hand writing recognition Januari project, MSc AI, University of Amsterdam
// Thijs Kooi, 2011

// The Baum-Welch algorithm, an instance of the EM-algorithm, is used to train the output probabilities
// To output the most likely sequence, the Viterbi algorithm, a dynamic programming method, is applied.

#include "hmm.h"

int main()
{
	//Testing
	HMM model;
	model.setStates(4);
	model.setObservations(4);
	model.initialiseUniform();
	
	cout << "Observations: " << model.getObservations() << endl;
	cout << "State: " << model.getStates() << endl;
	
	//Forward model initial probabilities
	double A[4][4] = 	{
				{0.5,0.5,0.0,0.0},
				{0.0,0.5,0.5,0.0},
				{0.0,0.0,0.5,0.5},
				{0.0,0.0,0.0,1.0},
				};
				
	//Test sequence, fully connected model
	double test_sequence[6] = {0,3,2,1,2,2};
	//Test sequence, forward model
// 	double test_sequence[6] = {0,1,1,2,2,3};

	model.printPriorProbabilities();
	model.printTransitionProbabilities();
	model.printObservationProbabilities();
	
	vector<int> test_sequence_vector;
	for(size_t i = 0; i < 6; ++i)
		test_sequence_vector.push_back(test_sequence[i]);
	
	cout << "Sequence probability: " << model.sequenceProbability(test_sequence_vector) << endl;
	
// 	double observations[] = {2,3,4,5,3,1,4};
// 	model.trainModel(observations);
// 	model.printObservations();

	
}

//Constructors
HMM::HMM() { }

HMM::HMM(int ns, int no, map<int, map<int, double> > t, map<int, map<int, double> > o)
{
	number_of_states = ns;
	number_of_observations = no;
	transition_probabilities = t;
	observation_probabilities = o;
}
//End constructors

//Getters and setters
int HMM::getStates(){ return number_of_states;  }
int HMM::getObservations(){ return number_of_observations; }
void HMM::setStates(int s){ number_of_states = s;  }
void HMM::setObservations(int o){ number_of_observations = o; }
//End getters and setters

//Training functions
void HMM::trainModel(double* o)
{
	observations = o;
	cout << "Training model..." << endl;
	int converged=0;
	initialiseParameters();
	while(!converged)
	{
		eStep();
		mStep();
		converged =1;
	}
	cout << "Done!" << endl;
}

void HMM::initialiseUniform() { initialiseParameters(); }

void HMM::initialiseParameters()
{
	//Initialise elements of transition map
	for(size_t i = 0; i < number_of_states; ++i)
		for(size_t j = 0; j < number_of_states; ++j)
			transition_probabilities[i][j] = 1.0;
		
	//Initialise uniform probabilities
	for(map<int, map<int,double> >::iterator i = transition_probabilities.begin(); i != transition_probabilities.end(); ++i)
		for(map<int, double>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
			(*j).second/=(*i).second.size();
		
	prior_probabilities = new double[number_of_states];
	//Initialise uniform prior probabilities
	for(size_t i = 0; i < number_of_states; ++i)
		prior_probabilities[i] = 1.0/number_of_states;
	
	//Initialise uniform probabilities for observations
	for(size_t i = 0; i < number_of_states; ++i)
		for(size_t j = 0; j < number_of_observations; ++j)
			observation_probabilities[i][j] = 1.0;
		
	for(map<int, map<int,double> >::iterator i = observation_probabilities.begin(); i != observation_probabilities.end(); ++i)
		for(map<int, double>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
			(*j).second/=(*i).second.size();
	
}

//Find posterior distribution of the latent variables
void HMM::eStep() { current_likelihood = forwardProbability(number_of_states,number_of_observations); }

//Generally denoted alpha in the literature
//Please note that the literature typically numbers states and observations 1-N, 1-K respectively,
//whereas the programming language starts enumerating at 0
double HMM::forwardProbability(int state, int timestep)
{
	if(state == 0 && timestep == 0)
		return 1.0;
	else if(timestep == 0)
		return transition_probabilities[0][state]*observation_probabilities[state][observations[0]];
	
	//Sum over all N-1 states
	double sum = 0.0;
	for(size_t i = 1; i < number_of_states-1; ++i)
		sum+=forwardProbability(i,timestep-1)*transition_probabilities[i][state];
		
	return observation_probabilities[state][observations[timestep]];
}

//Generally denoted beta in the literature
double HMM::backwardProbability(int state, int timestep)
{
	double sum = 0.0;
	
	if(timestep == number_of_observations)
		for(size_t j = 1; j < number_of_states-1; ++j)
			sum+=transition_probabilities[1][j]*observation_probabilities[j][observations[0]]*backwardProbability(j,timestep+1);
	else
		for(size_t j = 1; j < number_of_states-1; ++j)
			sum+=transition_probabilities[1][j]*observation_probabilities[j][observations[timestep]]*backwardProbability(j,timestep+1);
	
	return sum;
}

void HMM::mStep()
{
	maximisePriors();
	maximiseTransitions();
	maximiseObservationDistribution();
}

void HMM::maximisePriors()
{
}

void HMM::maximiseTransitions()
{
	for(map<int, map<int,double> >::iterator i = transition_probabilities.begin(); i != transition_probabilities.end(); ++i)
		for(map<int,double>::iterator j = i->second.begin(); j != i->second.end(); ++j)
			updateTransition(i->first,j->first);
}

void HMM::updateTransition(int i, int j)
{
	double numerator,denominator,sum;
	
	sum = 0.0;
	for(size_t t = 0; t < number_of_observations-1; ++t)
		sum += (forwardProbability(i,t)*transition_probabilities[i][j]*observation_probabilities[j][observations[t+1]]*backwardProbability(j,t+1));
	numerator = (1.0/current_likelihood)*sum;
		
	sum = 0.0;
	for(size_t t = 0; t < number_of_observations-1; ++t)
		sum+=(forwardProbability(i,t)*backwardProbability(i,t));
	denominator = (1.0/current_likelihood)*sum;
	
	transition_probabilities[i][j] = numerator/denominator;
}

void HMM::maximiseObservationDistribution()
{
}
//End training functions

//Model properties
//Returns the likelihood of the observation sequence
double HMM::likelihood() { return current_likelihood; }
double HMM::sequenceProbability(vector<int> sequence)
{
	double probability = prior_probabilities[sequence[0]];
	for(size_t i = 1; i < sequence.size(); ++i)
		probability*=transition_probabilities[sequence[i-1]][sequence[i]];
	
	return probability;
}
//End properties




//Testing and debugging
void HMM::printObservations()
{
	cout << "Training on observations: " << endl;
	for(size_t i = 0; i < number_of_observations; ++i)
		cout << observations[i] << " ";
	cout << endl;
}
void HMM::printPriorProbabilities()
{
	cout << "Current prior probabilities: " << endl;
	for(size_t i = 0; i < number_of_states; ++i)
		cout << prior_probabilities[i] << " ";
	cout << endl;
}
void HMM::printTransitionProbabilities()
{
	cout << "Current transition probabilities: " << endl;
	for(map<int, map< int, double> >::iterator i = transition_probabilities.begin(); i != transition_probabilities.end(); ++i)
	{
		for(map<int, double>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
			cout << (*j).second << " ";
		cout << endl;
	}
}
void HMM::printObservationProbabilities()
{
	cout << "Current observation probabilities: " << endl;
	for(map<int, map< int, double> >::iterator i = observation_probabilities.begin(); i != observation_probabilities.end(); ++i)
	{
		for(map<int, double>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
			cout << (*j).second << " ";
		cout << endl;
	}
}
//End testing and debugging