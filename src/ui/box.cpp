#include "box2dtest.h"
#include "imgui.h"
//#define DEMO
#ifdef DEMO
#include "scenarios/ScenarioHikeEval.h"
#include "scenarios/ScenarioImitateEval.h"
#include "scenarios/ScenarioImitateStepEval.h"
#include "scenarios/ScenarioImitateTargetEval.h"
#include "scenarios/ScenarioPoliEval.h"
#include "scenarios/ScenarioSoccerEval.h"
#include "sim/WaypointController.h"
#include "util/ArgParser.h"
#include "util/FileUtil.h"
#include <LinearMath/btIDebugDraw.h>
extern btIDebugDraw *debugDraw;
#else
#include <LinearMath/btTransform.h>
#include <LinearMath/btVector3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#endif
//======
static int state_scroll = 0, save_scroll = 0;
static bool state_show = false;
static int ctrl = 0;
static int touch = 0;
static int running = 1;
static int subStep = 1;
bool inv_motor = false;
bool over = false;
struct b2Vec2 {
  b2Vec2(float _x, float _y) {
    x = _x;
    y = _y;
  }
  float x, y;
};
b2Vec2 point(0, 0), old(0, 0);
float angle = 0;
float rotate = 0.0625;
extern float world[16];
bool down = false;
void MouseDown(b2Vec2 &m) {
  old = m;
  down = true;
}
void MouseUp(b2Vec2 &m) {
  down = false;
}
void MouseMove(b2Vec2 &m) {
  if (down) {
    b2Vec2 n(m.x - old.x, m.y - old.y);
    angle += n.x * 0.0075f;
    world[13] += (old.y - m.y) * 0.0015f;
    world[2] = -sinf(angle);
    world[0] = cosf(angle);
    world[8] = -world[2];
    world[10] = world[0];
    old = m;
  }
}
#ifdef DEMO
std::shared_ptr<cArgParser> gArgParser = nullptr;
std::shared_ptr<cScenarioSimChar> gScenario = nullptr;
float time_step = 0.0333f;
int selectId = 0;
const char *ARGS[] = {"data/args/soccer_eval_args.txt",  "data/args/hlc_dyna_eval_args.txt",    "data/args/llc_eval_args_bin.txt",
                      "data/args/hlc_eval_args.txt",     "data/args/hlc_pillars_eval_args.txt", "data/args/hlc_block_eval_args.txt",
                      "data/args/hlc_roll_eval_args.txt"};
const char *ARGS_name[] = {"soccer_eval", "hlc_dyna_eval", "llc_eval", "hlc_trail3d_eval", "hlc_pillars_eval", "hlc_block_eval", "hlc_roll_eval"};
void ParseArgs(int argc, char **argv) {
  char *_argv[2];
  //"data/args/hlc_dyna_eval_args.txt"
  //#define ARGS "data/args/llc_eval_args_bin.txt"
  char two[2][256];
  strcpy(two[0], "-arg_file=");
  strcpy(two[1], ARGS[selectId]);
  _argv[0] = two[0];
  _argv[1] = two[1];
  int _argc = 2;
  gArgParser = std::shared_ptr<cArgParser>(new cArgParser(_argv, _argc));
  std::string arg_file = "";
  gArgParser->ParseString("arg_file", arg_file);
  if (arg_file != "") {
    // append the args from the file to the ones from the commandline
    // this allows the cmd args to overwrite the file args
    gArgParser->AppendArgs(arg_file);
  }
}
extern float position[3];
cScenarioExpImitateStep *p = NULL;
clock_t preTime;
void box2d_init() {
  ParseArgs(0, 0);
  std::string scenario_name = "";
  gArgParser->ParseString("scenario", scenario_name);
  if (scenario_name == "sim_char") {
    gScenario = std::shared_ptr<cScenarioSimChar>(new cScenarioSimChar());
  } else if (scenario_name == "track_motion") {
    // gScenario = std::shared_ptr<cScenarioSimChar>(new cScenarioTrackMotion());
  } else if (scenario_name == "train") {
    // gScenario = std::shared_ptr<cScenarioSimChar>(new cDrawScenarioTrain(gCamera));
  } else if (scenario_name == "train_cacla") {
    // gScenario = std::shared_ptr<cScenarioSimChar>(new cDrawScenarioTrainCacla(gCamera));
  } else if (scenario_name == "train_cacla_dq") {
    // gScenario = std::shared_ptr<cScenarioSimChar>(new cDrawScenarioTrainCacla(gCamera));
  } else if (scenario_name == "imitate") {
    // gScenario = std::shared_ptr<cScenarioSimChar>(new cDrawScenarioImitate(gCamera));
  } else if (scenario_name == "imitate_target") {
    // gScenario = std::shared_ptr<cScenarioSimChar>(new cDrawScenarioImitateTarget(gCamera));
  } else if (scenario_name == "imitate_step") {
    // gScenario = std::shared_ptr<cScenarioSimChar>(new cDrawScenarioImitateStep(gCamera));
  } else if (scenario_name == "train_hike") {
    // gScenario = std::shared_ptr<cScenarioSimChar>(new cDrawScenarioTrainHike(gCamera));
  } else if (scenario_name == "train_soccer") {
    // gScenario = std::shared_ptr<cScenarioSimChar>(new cDrawScenarioTrainSoccer(gCamera));
  } else if (scenario_name == "poli_eval") {
    // gScenario = std::shared_ptr<cScenarioSimChar>(new cDrawScenarioPoliEval(gCamera));
  } else if (scenario_name == "imitate_eval") {
    gScenario = std::shared_ptr<cScenarioSimChar>(new cScenarioImitateEval());
  } else if (scenario_name == "imitate_target_eval") {
    gScenario = std::shared_ptr<cScenarioSimChar>(new cScenarioImitateTargetEval());
  } else if (scenario_name == "imitate_step_eval") {
    gScenario = std::shared_ptr<cScenarioSimChar>(new cScenarioImitateStepEval());
  } else if (scenario_name == "hike_eval") {
    gScenario = std::shared_ptr<cScenarioSimChar>(new cScenarioHikeEval());
  } else if (scenario_name == "soccer_eval") {
    gScenario = std::shared_ptr<cScenarioSimChar>(new cScenarioSoccerEval());
  }
  p = dynamic_cast<cScenarioExpImitateStep *>(gScenario.get());
  /*cWaypointController *ct = dynamic_cast<cWaypointController *>(gScenario->GetCharacter()->GetController().get());
  if (ct)
    ct->EnableSymmetricStep(true);*/
  // gScenario = std::shared_ptr<cScenarioSimChar>(p = new cScenarioImitateStepEval());
  gScenario->ParseArgs(gArgParser);
  gScenario->Init();
  preTime = clock();
  const auto &pos = gScenario->GetCharPos();
  position[0] = pos[0];
  position[1] = pos[1];
  position[2] = pos[2];
}
void box2d_step() {
  if (running) {
    /*auto now = clock();
    time_step = btMin((now - preTime) * 0.000001, 0.0425);
    preTime = now;*/
    gScenario->Update(time_step);
    const auto &pos = gScenario->GetCharPos();
    position[0] = pos[0];
    position[1] = pos[1];
    position[2] = pos[2];
  }
}
void box2d_draw() {
  gScenario->Draw();
  if (p) {
    const auto &step = p->GetStepPlan();
    const auto &pos0 = step.mStepPos0;
    const auto &pos1 = step.mStepPos1;
    debugDraw->drawSphere(btVector3(pos0[0] - position[0], pos0[1], pos0[2] - position[2]), 0.025, btVector3(1, 0.75, 0.25));
    debugDraw->drawSphere(btVector3(pos1[0] - position[0], pos1[1], pos1[2] - position[2]), 0.025, btVector3(1, 0.75, 0.25));
    // debugDraw->drawSphere(btVector3(pos[0], pos[1], pos[2]), 0.05, btVector3(1, 0.25, 0.5));
    if (p->EnableTargetPos()) {
      const auto &pos = p->GetTargetPos();
      debugDraw->drawSphere(btVector3(pos[0] - position[0], pos[1], pos[2] - position[2]), 0.075, btVector3(1, 0.25, 0.25));
    }
  }
}
#else
std::vector<float> pos;
std::vector<float> nor;
std::vector<float> nnn;
std::vector<unsigned short> ind;
btTransform localMtx;
float box[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 4};
// float ball[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 4};
void *makeNet();
std::vector<float> &getValue(void *net, std::vector<float> &input);
int trainNet(void *net, const std::vector<float> &input, const std::vector<float> target);
void *net = 0;
void box2d_init() {
  srand48(time(0));
  net = makeNet();
  unsigned short st = pos.size() / 3;
  unsigned short stt = st;
  for (int x = 0; x < 7; ++x) {
    for (int y = 0; y < 7; ++y) {
      pos.push_back((x - 3) * 0.2);
      pos.push_back(3.0 * 0.2);
      pos.push_back((y - 3) * 0.2);
      nor.push_back(0);
      nor.push_back(1);
      nor.push_back(0);
      if (x < 6 && y < 6) {
        unsigned short f = x + y * 7 + st;
        ind.push_back(f);
        ind.push_back(f + 1);
        ind.push_back(f + 7);
        ind.push_back(f + 7);
        ind.push_back(f + 1);
        ind.push_back(f + 8);
      }
    }
  }
  st = pos.size() / 3;
  for (int x = 0; x < 7; ++x) {
    for (int y = 0; y < 7; ++y) {
      pos.push_back((x - 3) * -0.2);
      pos.push_back(-3.0 * 0.2);
      pos.push_back((y - 3) * 0.2);
      nor.push_back(0);
      nor.push_back(-1);
      nor.push_back(0);
      if (x < 6 && y < 6) {
        unsigned short f = x + y * 7 + st;
        ind.push_back(f);
        ind.push_back(f + 1);
        ind.push_back(f + 7);
        ind.push_back(f + 7);
        ind.push_back(f + 1);
        ind.push_back(f + 8);
      }
    }
  }
  st = pos.size() / 3;
  for (int x = 0; x < 7; ++x) {
    for (int y = 0; y < 7; ++y) {
      pos.push_back((x - 3) * -0.2);
      pos.push_back((y - 3) * 0.2);
      pos.push_back(3.0 * 0.2);
      nor.push_back(0);
      nor.push_back(0);
      nor.push_back(1);
      if (x < 6 && y < 6) {
        unsigned short f = x + y * 7 + st;
        ind.push_back(f);
        ind.push_back(f + 1);
        ind.push_back(f + 7);
        ind.push_back(f + 7);
        ind.push_back(f + 1);
        ind.push_back(f + 8);
      }
    }
  }
  st = pos.size() / 3;
  for (int x = 0; x < 7; ++x) {
    for (int y = 0; y < 7; ++y) {
      pos.push_back((x - 3) * 0.2);
      pos.push_back((y - 3) * 0.2);
      pos.push_back(-3.0 * 0.2);
      nor.push_back(0);
      nor.push_back(0);
      nor.push_back(-1);
      if (x < 6 && y < 6) {
        unsigned short f = x + y * 7 + st;
        ind.push_back(f);
        ind.push_back(f + 1);
        ind.push_back(f + 7);
        ind.push_back(f + 7);
        ind.push_back(f + 1);
        ind.push_back(f + 8);
      }
    }
  }
  st = pos.size() / 3;
  for (int x = 0; x < 7; ++x) {
    for (int y = 0; y < 7; ++y) {
      pos.push_back(3.0 * 0.2);
      pos.push_back((y - 3) * 0.2);
      pos.push_back((x - 3) * 0.2);
      nor.push_back(1);
      nor.push_back(0);
      nor.push_back(0);
      if (x < 6 && y < 6) {
        unsigned short f = x + y * 7 + st;
        ind.push_back(f);
        ind.push_back(f + 1);
        ind.push_back(f + 7);
        ind.push_back(f + 7);
        ind.push_back(f + 1);
        ind.push_back(f + 8);
      }
    }
  }
  st = pos.size() / 3;
  for (int x = 0; x < 7; ++x) {
    for (int y = 0; y < 7; ++y) {
      pos.push_back(-3.0 * 0.2);
      pos.push_back((y - 3) * -0.2);
      pos.push_back((x - 3) * 0.2);
      nor.push_back(-1);
      nor.push_back(0);
      nor.push_back(0);
      if (x < 6 && y < 6) {
        unsigned short f = x + y * 7 + st;
        ind.push_back(f);
        ind.push_back(f + 1);
        ind.push_back(f + 7);
        ind.push_back(f + 7);
        ind.push_back(f + 1);
        ind.push_back(f + 8);
      }
    }
  }
  st = pos.size() / 3;
  for (int i = stt; i < st; ++i) {
    float &x = pos[i * 3 + 0];
    float &y = pos[i * 3 + 1];
    float &z = pos[i * 3 + 2];
    float d = 1.0f / sqrtf(x * x + y * y + z * z);
    x *= d;
    y *= d;
    z *= d;
    nor[i * 3 + 0] = x;
    nor[i * 3 + 1] = y;
    nor[i * 3 + 2] = z;
  }
  nnn = nor;
  localMtx.setIdentity();
}
static std::vector<float> input(96), target(96);
void box2d_step() {
  auto tmp = getValue(net, nor);
  int ct = pos.size() / 3;
  if (ct == tmp.size() / 3) {
    /*for (int i = 0; i < ct; ++i) {
      float &x = pos[i * 3 + 0];
      float &y = pos[i * 3 + 1];
      float &z = pos[i * 3 + 2];
      float &nx = nor[i * 3 + 0];
      float &ny = nor[i * 3 + 1];
      float &nz = nor[i * 3 + 2];
      x = x * 0.92 + 0.08 * tmp[i * 3 + 0];
      y = y * 0.92 + 0.08 * tmp[i * 3 + 1];
      z = z * 0.92 + 0.08 * tmp[i * 3 + 2];
    }*/ pos = tmp;
  }
  static int step = 0;
  static int nx=0;
  if (step <= 0) {
    int cct = ct;
     int ii = nx;
    /*for (int ii = 0; ii < 32; ++ii)*/ {
      ct = rand() % cct;
      float &xx = nor[ct * 3 + 0];
      float &yy = nor[ct * 3 + 1];
      float &zz = nor[ct * 3 + 2];
      float mx = 1.0 / fmax(fmaxf(fabsf(xx), fabsf(yy)), fabsf(zz));
      input[ii * 3 + 0] = xx;
      input[ii * 3 + 1] = yy;
      input[ii * 3 + 2] = zz;
      target[ii * 3 + 0] = xx * mx;
      target[ii * 3 + 1] = yy * mx;
      target[ii * 3 + 2] = zz * mx;
    }
    //ii = (ii + 1) % 32;
    // step = 10 + rand() % 50;
  } else
    --step;
  nx=trainNet(net, input, target);
}
extern "C" void drawMesh(const float *m, const void *v, const void *n, const void *i, int size, const btVector3 c);
void box2d_draw() {
  if (ind.size() > 0) {
    int fc = ind.size() / 3;
    float *p = pos.data();
    memset(nnn.data(), 0, sizeof(float) * nnn.size());
    btVector3 v;
    for (int i = 0; i < fc; ++i) {
      unsigned short d0 = ind[i * 3 + 0];
      unsigned short d1 = ind[i * 3 + 1];
      unsigned short d2 = ind[i * 3 + 2];
      btVector3 v0(pos[d0 * 3 + 0], pos[d0 * 3 + 1], pos[d0 * 3 + 2]);
      btVector3 v1(pos[d1 * 3 + 0], pos[d1 * 3 + 1], pos[d1 * 3 + 2]);
      btVector3 v2(pos[d2 * 3 + 0], pos[d2 * 3 + 1], pos[d2 * 3 + 2]);
      v = btCross(v1 - v0, v2 - v0);
      nnn[d0 * 3 + 0] += v.x();
      nnn[d0 * 3 + 1] += v.y();
      nnn[d0 * 3 + 2] += v.z();
      v = btCross(v2 - v1, v0 - v1);
      nnn[d1 * 3 + 0] += v.x();
      nnn[d1 * 3 + 1] += v.y();
      nnn[d1 * 3 + 2] += v.z();
      v = btCross(v0 - v2, v1 - v2);
      nnn[d2 * 3 + 0] += v.x();
      nnn[d2 * 3 + 1] += v.y();
      nnn[d2 * 3 + 2] += v.z();
    }
    fc = nnn.size() / 3;
    for (int i = 0; i < fc; ++i) {
      float &x = nnn[i * 3 + 0];
      float &y = nnn[i * 3 + 1];
      float &z = nnn[i * 3 + 2];
      float d = 1.0f / sqrtf(x * x + y * y + z * z);
      x *= d;
      y *= d;
      z *= d;
    }
    {
      box[2] = -sinf(rotate);
      box[0] = cosf(rotate);
      box[8] = -box[2];
      box[10] = box[0];
      localMtx.setFromOpenGLMatrix(box);
    }
    btVector3 c = {1, 1, 1};
    box[15] = 4;
    box[12] = box[13] = box[14] = 0;
    drawMesh(box, pos.data(), nnn.data(), ind.data(), ind.size(), c);
    box[15] = 50;
    for (int ii = 0; ii < 32; ++ii) {
      btVector3 vt(target[ii * 3 + 0] * 50 / 4, target[ii * 3 + 1] * 50 / 4, target[ii * 3 + 2] * 50 / 4);
      vt = localMtx.getBasis() * vt;
      box[12] = vt.x();
      box[13] = vt.y();
      box[14] = vt.z();
      drawMesh(box, nor.data(), nor.data(), ind.data(), ind.size(), c);
    }
  }
#define PLANE_SIZE 10
  static float mtx[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, -2, 0, 4};
  static float plane[18] = {-PLANE_SIZE, 0, -PLANE_SIZE, -PLANE_SIZE, 0, PLANE_SIZE, PLANE_SIZE, 0, PLANE_SIZE,
                            -PLANE_SIZE, 0, -PLANE_SIZE, PLANE_SIZE,  0, PLANE_SIZE, PLANE_SIZE, 0, -PLANE_SIZE};
  static float normal[18] = {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0};
  static btVector3 color = {0.4, 0.6, 0.8};
  drawMesh(mtx, plane, normal, 0, 6, color);
}
int btMin(int a, int b) {
  return a < b ? a : b;
}
#endif
#define USE_UI
namespace ui {
static const int BUTTON_HEIGHT = 80;
static const int SLIDER_HEIGHT = 80;
static const int SLIDER_MARKER_WIDTH = 40;
static const int CHECK_SIZE = 32;
static const int DEFAULT_SPACING = 16;
static const int TEXT_HEIGHT = 32;
static const int SCROLL_AREA_PADDING = 24;
static const int INDENT_SIZE = 64;
static const int AREA_HEADER = 112;
} // namespace ui
int stateSize = 0, inputSize = 0;
void box2d_ui(int width, int height, int mx, int my, unsigned char mbut, int scroll) {
#if 0
    if (mbut) {
    if (ctrl > 0) {
      mbut = 0;
    }
    ctrl = 0;
    ++touch;
#ifdef USE_UI
    if (over || mbut == 0)
      imguiBeginFrame(mx, height - my, mbut, scroll);
    else
      imguiBeginFrame(0, 0, mbut, scroll);
#endif
  } else {
    ++ctrl;
    touch = 0;
    over = false;
#ifdef USE_UI
    imguiBeginFrame(0, 0, mbut, scroll);
#endif
  }
#else
  
  if (mbut) {
    imguiBeginFrame(mx, height - my, mbut, scroll);
    ctrl = 0;
    ++touch;
  } else {
    imguiBeginFrame(0, 0, 0, scroll);
    touch = 0;
    over = false;
    ++ctrl;
  }
#endif
#ifdef USE_UI
  char info[260];
  sprintf(info, "DeepLoco: %d ,%d", inputSize, stateSize);
  int size = (ui::AREA_HEADER + ui::BUTTON_HEIGHT * 3 + ui::DEFAULT_SPACING * 2 + ui::SCROLL_AREA_PADDING);
  #ifndef DEMO
  size += ui::BUTTON_HEIGHT + ui::DEFAULT_SPACING;
  #endif
  if (state_show)
    size += ui::BUTTON_HEIGHT * 3 + ui::DEFAULT_SPACING * 2; // btMax(width, height) * 3 / 8;
  over |= imguiBeginScrollArea(info, width - btMin(width, height) * 2 / 3 - (width > height ? 120 : 0), height - size - 60, btMin(width, height) * 2 / 3 - 10,
                               size, &state_scroll);
  if (imguiButton("Reset", true)) {
#ifdef DEMO
    gScenario->Reset();
    const auto &pos = gScenario->GetCharPos();
    position[0] = pos[0];
    position[1] = pos[1];
    position[2] = pos[2];
#else
    net = makeNet();
#endif
  }
  if (imguiButton("Pause", true)) {
    running = !running;
  }
  /*if (imguiButton("Train", true)) {
    inv_motor = true;
  }*/
  // imguiLabel("Label");
  #ifndef DEMO
  imguiSlider("rotate", &rotate, 0, 0.5, 0.001, true);
  #endif
  if (imguiCollapse("Items", 0, state_show, true)) {
    if (state_show) {
      save_scroll = state_scroll;
      state_scroll = 0;
    } else {
      state_scroll = save_scroll;
    }
    state_show = !state_show;
  }
  if (state_show) {
    for (int i = 0; i < 7; ++i) {
#ifdef DEMO
      if (imguiItem(ARGS_name[i], selectId != i)) {
        selectId = i;
        box2d_init();
      }
#else
      imguiItem("ARGS_name[i]", true);
#endif
    }
  }
  imguiEndScrollArea();
  imguiEndFrame();
#endif
  point = b2Vec2(mx, my);
  if (touch == 1) {
    if (!over)
      MouseDown(point);
  } else if (ctrl == 1) {
    MouseUp(point);
  } else {
    MouseMove(point);
  }
}
void box2d_quit() {
}
void box2d_gravity(float x, float y) {
}