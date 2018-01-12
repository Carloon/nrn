#include <pthread.h>
#include <matrix.h>
/*borrowed from Meschach Version 1.2b*/
#define	v_get_val(x,i)		((x)->ve[(i)])
#define	m_get_val(A,i,j)	((A)->me[(i)][(j)])
#define SPECIES_ABSENT      -1

typedef void (*fptr)(void);

typedef struct {
    PyObject_HEAD
	struct OcPtrVector (*newOcPtrVector)();
	void (*deleteOcPtrVector)();
	int (*size)(int);
	void (*resize)();
	void (*pset)(int, double*);
	double (*getval)(int);
	void (*setval)(int,double);
	void (*scatter)(double*,int);
	void (*gather)(double*, int);
	void (*ptr_update_cmd)(void*);
	void (*ptr_update)();
	void* update_cmd_;
	int size_;
	double ** pd_;
} OcPtrVector;		
/*
		OcPtrVector* newOcPtrVector();
		} ;
*/

typedef struct {
	Reaction* reaction;
	int idx;
} ReactSet;

typedef struct {
	ReactSet* onset;
	ReactSet* offset;
} ReactGridData;

typedef struct {
        double* copyFrom;
        long copyTo;
} AdiLineData;

typedef struct {
    int start, stop;
    AdiLineData* vals;
    double* state;
    Grid_node g;
    int sizej;
    AdiLineData (*dg_adi_dir)(Grid_node, double, int, int, double const *, double*);
    double* scratchpad;
} AdiGridData;

typedef struct ICSReactions{
    ReactionRate reaction;
    int num_species;
    int num_regions;
    int num_segments;
    int*** state_idx;   /*[segment][species][region]*/
    int icsN;   /*total number species*regions per segment*/
    /*NOTE: icsN != num_species*num_regions as every species may not be defined
     *on every region - the missing elements of state_idx are SPECIES_ABSENT*/

    /*ECS for MultiCompartment reactions*/
    int num_ecs_species;
    double**** ecs_state;    /*[segment][ecs_species][region]*/
    int ecsN;    /*total number of ecs species*regions per segment*/
    
    int num_mult;
    double **mc_multiplier;
    struct ICSReactions* next;
} ICSReactions;

typedef struct {
    /*variables for reactions*/
    double* states_for_reaction;
    double* states_for_reaction_dx;
    double* ecs_states_for_reaction;
    double* ecs_states_for_reaction_dx;
	double* result_array;
	double* result_array_dx;
	double* result_ecs;
	double* result_ecs_dx;
    MAT *jacobian;
    VEC *x;
    VEC *b;
    PERM *pivot;

} ReactionVariables;

void set_num_threads(int);
void _fadvance(void);
void _fadvance_fixed_step_ecs(void);

int get_num_threads(void);
static int dg_adi(Grid_node);
int dg_adi_vol(Grid_node);
int dg_adi_tort(Grid_node);
void dg_transfer_data(AdiLineData * const, double* const, int const, int const, int const);
void run_threaded_dg_adi(AdiGridData*, pthread_t*, const int, const int, Grid_node, double*, AdiLineData*, AdiLineData (*dg_adi_dir)(Grid_node, double, int, int, double const *, double*), const int n);

ReactGridData* create_threaded_reactions(void);
void* do_reactions(void*);

typedef void (*fptr)(void);
void current_reaction(double *states);


/*Variable step function declarations*/
void _rhs_variable_step(const double, const double*, double*);

void _ode_reinit(double*);

int ode_count(const int);

void scatter_concentrations(void);

static void update_boundaries_x(int i, int j, int k, int dj, int dk, double rate_x,
 double rate_y, double rate_z, int num_states_x, int num_states_y, int num_states_z,
 const double const* states, double* ydot);


static void update_boundaries_y(int i, int j, int k, int di, int dk, double rate_x,
 double rate_y, double rate_z, int num_states_x, int num_states_y, int num_states_z,
 const double const* states, double* ydot);

static void update_boundaries_z(int i, int j, int k, int di, int dj, double rate_x,
 double rate_y, double rate_z, int num_states_x, int num_states_y, int num_states_z,
 const double const* states, double* ydot);

static void _rhs_variable_step_helper(Grid_node* grid, const double const* states, double* ydot);

int find(const int, const int, const int, const int, const int);

void _rhs_variable_step_helper_tort(Grid_node*, const double const*, double*);

void _rhs_variable_step_helper_vol(Grid_node*, const double const*, double*);

void ecs_refresh_reactions(int);

void _rhs_variable_step_ecs(const double, const double*, double*);

void clear_rates_ecs();
void do_ics_reactions(const double, const double*, double*);
void _ecs_ode_reinit(double*); 
void do_currents(Grid_node*, double*, double);
