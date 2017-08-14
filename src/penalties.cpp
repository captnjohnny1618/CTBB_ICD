#include <vector>
#include <math.h>

#include "penalties.h"

#define SQRT2 1.41421356237309504880168872420969807

inline int sign(double x){
    if (x<0)
        return -1;
    else
        return 1;
}

double func(double du, double alpha, double beta, double delta, double lambda, std::vector<double> &recon_volume,
            double *wgts, std::size_t*neighbor_indices, int q, const int num_neighbors);

void initialize_2d_weights(struct iterative_params *ip){

    ip->num_neighbors=8;
    
    ip->weights=new double[ip->num_neighbors];

    ip->weights[0] = 1.0 / SQRT2;
    ip->weights[1] = 1.0;
    ip->weights[2] = 1.0 / SQRT2;
    ip->weights[3] = 1.0;
    ip->weights[4] = 1.0;
    ip->weights[5] = 1.0 / SQRT2;
    ip->weights[6] = 1.0;
    ip->weights[7] = 1.0 / SQRT2;

    ip->weights_scale = 0.0;
    for (int i = 0; i < ip->num_neighbors; i++){
        ip->weights_scale += ip->weights[i];
    }
    
}

double quadratic(int curr_idx,struct iterative_params * ip, std::vector<double> &recon_volume){
    // Compute pixel update using 2D (In-plane) QUADRATIC PENALTY function
    
    // Identify indices of voxels we'll compute 
    std::size_t neighbor_indices[8];
    
    neighbor_indices[0] = curr_idx - ip->Nx - 1;
    neighbor_indices[1] = curr_idx - ip->Nx;
    neighbor_indices[2] = curr_idx - ip->Nx + 1;
    neighbor_indices[3] = curr_idx - 1;
    neighbor_indices[4] = curr_idx + 1;
    neighbor_indices[5] = curr_idx + ip->Nx - 1;
    neighbor_indices[6] = curr_idx + ip->Nx;
    neighbor_indices[7] = curr_idx + ip->Nx + 1;

    double sum = 0.0;
    for (int i = 0; i < ip->num_neighbors; i++){
        sum += ip->weights[i] * (recon_volume[neighbor_indices[i]] - recon_volume[curr_idx]);
    }

    // Return amount by which pixel will be updated
    return  (ip->beta + ip->lambda*sum) / (ip->alpha + ip->lambda*ip->weights_scale);
}


double edge_preserving(int curr_idx,struct iterative_params * ip, std::vector<double> &recon_volume){
   // Identify indices of voxels we'll compute 
    std::size_t neighbor_indices[8];
    
    neighbor_indices[0] = curr_idx - ip->Nx - 1;
    neighbor_indices[1] = curr_idx - ip->Nx;
    neighbor_indices[2] = curr_idx - ip->Nx + 1;
    neighbor_indices[3] = curr_idx - 1;
    neighbor_indices[4] = curr_idx + 1;
    neighbor_indices[5] = curr_idx + ip->Nx - 1;
    neighbor_indices[6] = curr_idx + ip->Nx;
    neighbor_indices[7] = curr_idx + ip->Nx + 1;

    if (ip->alpha == 0.0)
        return 0.0; // TODO: should return sum1/sum2 for consistency with quadratic penalty

    double a = ip->beta / ip->alpha;
    double b = ip->beta / ip->alpha;

    for (int i = 0; i<ip->num_neighbors; i++){
        double diff = (recon_volume[neighbor_indices[i]] - recon_volume[curr_idx]);
        
        if (diff < a)
            a = diff;
        if (diff > b)
            b = diff;
    }

    int max_iterations = 1000000;
    double tol = 0.00000001;

    for (int i=0; i<max_iterations; i++){

        double c = (a+b)/2.0;

        double f_c=func(c,ip->alpha,ip->beta,ip->delta,ip->lambda,recon_volume,ip->weights,neighbor_indices,curr_idx,ip->num_neighbors);
        double f_a=func(a,ip->alpha,ip->beta,ip->delta,ip->lambda,recon_volume,ip->weights,neighbor_indices,curr_idx,ip->num_neighbors);

        if (( f_c == 0) || (b-a)/2<tol)            
            return c;        
        else{
            if (sign(f_c)==sign(f_a))
                a=c;            
            else
                b=c;            
        }
    }    
}

double func(double du, double alpha, double beta, double delta, double lambda, std::vector<double> &recon_volume,
            double *wgts, std::size_t *neighbor_indices, int q, int num_neighbors){
	double sum = 0;
	for (int i = 0; i < num_neighbors; i++){
		double t = (du - (recon_volume[neighbor_indices[i]] - recon_volume[q]));
		if (t >= 0)
			sum = sum + wgts[i] * (1.0 - 1.0 / (1.0 + fabs(t/delta)));
		else
			sum = sum - wgts[i] * (1.0 - 1.0 / (1.0 + fabs(t/delta)));
	}
	return 2 * alpha*du - 2 * beta + lambda*sum;
}
