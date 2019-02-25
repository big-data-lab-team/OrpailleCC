#include <cmath>
#include <cassert>

/**
 * A class that implement the reservoir sampling algorithm.
 *
 * The templates element_type indicate what type of element the class should sample, sample_size inidcate the size of the sample and random_function
 * give the class a random function that should uniformly return values between 0 and 1.
 */
template<class element_type, unsigned int sample_size, double (*random_function)(), class funct>
class ChainedReservoirSampling{
	struct node {
		node* next;
		element_type element;
		unsigned int timestamp;
	};
	node sample[sample_size];
	//`next` is not inside the node structure because once next is used for a node, it is not used again
	unsigned int next[sample_size];
	unsigned int counter = 0;

	/**
	 * Round d to the closest integer
	 * @param d the double to round
	 */
	double round(double const d) {
		  return (double)((int)(d + 0.5));
	}
	/**
	 * Function to reset a chain when it is replaced. This is a recursive function.
	 * @param start The node to start from.
	 */
	void clear_chain(node* start){
		if(start == NULL)
			return;
		start->timestamp = 0;
		//TODO improve this recursive code lazy boy -_-"
		clear_chain(start->next);
	}
	/** Add a new element in the first available space of a chain.
	 * If there is no space (and no loop) it run the malloc function given in `funct` to allocate a new space.
	 * @param head the head of the chain to add to.
	 * @param e the element to add.
	 * @param timestamp the timestam of the element.
	 */
	void push_on_chain(node& head, element_type e, unsigned int const timestamp){
		node* current = &head;
		int i = 0;
		//The only thing that could make this loop forever is a corruption the linked list of node that loop over the same data without empty space.
		while(i < 5000){ //`i` is just a security
			if(current->timestamp == 0){ // This cell is available
				current->element = e;
				current->timestamp = timestamp;
				return;
			}
			if(current->next == NULL){
				node* new_element = static_cast<node*>(funct::malloc(sizeof(node)));
				assert(new_element != NULL);
				new_element->next = NULL;
				new_element->element = e;
				new_element->timestamp = timestamp;
				current->next = new_element;
				return;
			}

			current = current->next;
			i += 1;
		}
		assert(false);
		//TODO maybe clear the chain if it is corrupted
	}
	void shift_chain(node& head, node& current){
		node& receiver = head;
		node& mover = current;
		while(1){//TODO add security
			receiver.element = mover.element;
			receiver.timestamp = mover.timestamp;
			if(mover.next != NULL){
				mover = *mover.next;
				receiver = *receiver.next;
			}
			else
				break;
		}
		//Once everything is shifted, disable all following cells
		while(receiver.next != NULL){
			receiver = *receiver.next;
			receiver.timestamp = 0;
		}
	}
	public:
	/**
	 * Default constructor.
	 * Initialize an empty reservoir.
	 */
	ChainedReservoirSampling(){
		for(struct node& current : sample){
			current.next = NULL;
			current.timestamp = 0;
		}
		for(int i = 0; i < sample_size; ++i)
			next[i] = 4294967294; //`next` cannot be negative so to avoid 0 to be assign to all sample, I set it to the maximum
	}
	/**
	 * Sample one new element into the sample. This new element may not be added.
	 * @param e The new element to eventualy add to the sample.
	 */
	void add(element_type e, unsigned int const timestamp){
		if(counter < sample_size){
			sample[counter].element = e;
			sample[counter].timestamp = timestamp;
			sample[counter].next = NULL;
			next[counter] = counter + 1 + round(random_function() * (sample_size-1));
		}
		else{
			double const threshold = (double)sample_size / (double)counter;
			double const rnd = random_function();
			if(rnd < threshold){
				int const index = round(random_function() * (sample_size-1));
				sample[index].element = e;
				sample[index].timestamp = timestamp;
				clear_chain(sample[index].next); //NOTE we do not free the memory here, but it might be interesting to think about it
				next[index] = counter + 1 + round(random_function() * (sample_size-1));
			}
		}
		//check `next`
		for(int i = 0; i < sample_size; ++i){
			if(counter == next[i]){
				push_on_chain(sample[i], e, timestamp);
				next[i] = counter + 1 + round(random_function() * (sample_size-1));
			}
		}
		counter += 1;
	}
	/**
	 * A const operator to access the sample like an array.
	 * @param i the index to access.
	 */
	element_type const& operator[](int const i) const{
		return sample[i].element;
	}
	/**
	 * Operator to access the sample like an array.
	 * @param i the index to access.
	 */
	element_type& operator[](int const i) {
		return sample[i].element;
	}
	/*
	 * Declare a timestamp and all anterior timestamp obsolete
	 * @param timestamp the timestamp to declare obsolete
	 */
	/** Set a new obsolete timestamp. All element with a timestamp prior to this timestamp will be discarded.
	 * @param timestamp a new obsolete timestamp.
	 */
	void obsolete(unsigned int const timestamp){
		for(int i = 0; i < sample_size; ++i){
			node& head = sample[i];
			node* current = &head;

			//TODO make sure nothing get out of the chain
			while(current->timestamp < timestamp){
				assert(current->next != NULL);
				current = current->next;
			}
			if(&head != current)
				shift_chain(head, *current);
		}
	}
};

