template<int label_count>
class ErrorMetrics{
	int count = 0;
	int error_count = 0;
	public:
	void update(int const true_label, int const prediction){
		count += 1;
		error_count += (true_label != prediction);
	}
	double score(bool const invert=false, int const tree_size=-1) const{
		if(tree_size > 0 && !invert)
			return (static_cast<double>(error_count) / static_cast<double>(count)) * static_cast<double>(tree_size);
		else if(tree_size <= 0 && !invert)
			return (static_cast<double>(error_count) / static_cast<double>(count));
		else if(tree_size > 0 && invert)
			return (static_cast<double>(count - error_count) / static_cast<double>(count)) / static_cast<double>(tree_size);
		return (static_cast<double>(count - error_count) / static_cast<double>(count));
	}
	void increase_error(int const c=1){
		count += c;
		error_count += c;

	}
	void reset(){
		count = 0;
		error_count = 0;
	}
};
template<int label_count>
class KappaMetrics{
	int confusion[label_count][label_count];
	int total;
	public:
	KappaMetrics(){
		reset();
	}
	void operator=(KappaMetrics const& metric){
		if(&metric != this){
			for(int i = 0; i < label_count; ++i)
				for(int j = 0; j < label_count; ++j)
					confusion[i][j] = metric.confusion[i][j];
			total = metric.total;
		}
	}
	void update(int const true_label, int const prediction){
		confusion[true_label][prediction] += 1;
		total += 1;
	}
	template<bool pr=false>
	double kappa(void) const{
		double diaganol = 0;
		double sum_colrow = 0;
		for(int i = 0; i < label_count; ++i){
			diaganol += confusion[i][i];
			double sum_col = 0, sum_row = 0;
			for(int j = 0; j < label_count; ++j){
				sum_col += confusion[i][j];
				sum_row += confusion[j][i];
			}
			sum_colrow += sum_col * sum_row;
		}
		return (static_cast<double>(total) * diaganol - sum_colrow) / (static_cast<double>(total) * static_cast<double>(total) - sum_colrow);
	}
	double score(bool const invert = false, int const tree_size=-1) const{
		if(tree_size > 0 && invert)
			return ((kappa() + 1) / 2)/static_cast<double>(tree_size);
		else if(tree_size <= 0 && invert)
			return ((kappa() + 1) / 2);
		else if(tree_size > 0 && !invert)
			return (((kappa() * -1) + 1) / 2) * static_cast<double>(tree_size);
		return (((kappa() * -1) + 1) / 2);
	}
	void increase_error(int const c=1){
		confusion[0][1] += c;
		total += c;
	}
	void reset(){
		for(int i = 0; i < label_count; ++i)
			for(int j = 0; j < label_count; ++j)
				confusion[i][j] = 0;
		total = 0;
	}
};
class ReservoirSamplingMetrics{
	static int total_count;
	int number;
	public:
	ReservoirSamplingMetrics(){
		reset();
	}
	void operator=(ReservoirSamplingMetrics const& metric){
		if(&metric != this){
			number = metric.number;
		}
	}
	void update(int const true_label, int const prediction){
	}
	double score(bool const invert=false, int const sample_size=1) const{
		if(total_count == number)
			return static_cast<double>(number - sample_size) / static_cast<double>(total_count);
		return 1 / static_cast<double>(total_count);
	}
	void increase_error(int const c=1){
	}
	void reset(){
		total_count += 1;
		number = total_count;
	}
};
int ReservoirSamplingMetrics::total_count = 0;
