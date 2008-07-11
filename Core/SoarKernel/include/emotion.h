

typedef struct agent_struct agent;
typedef struct wme_struct wme;

void register_appraisal(agent* thisAgent, wme* appraisal);
void get_appraisals(agent* thisAgent);
void update_mood(agent* thisAgent);
void generate_feeling_frame(agent* thisAgent);
void emotion_reset(agent* thisAgent);