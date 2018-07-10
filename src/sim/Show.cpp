#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <LinearMath/btConvexHullComputer.h>
#include <btBulletDynamicsCommon.h>
#include <vector>
struct tMesh {
  std::vector<float> v, n;
  std::vector<unsigned short> i;
  int s;
};
float position[3];
extern "C" void drawMesh(const float *m, const void *v, const void *n, const void *i, int size, const btVector3 c);
void hideShape(btRigidBody *body) {
  btCollisionShape *shape = body->getCollisionShape();
  if (shape->isConvex()) {
    tMesh *m = (tMesh *)shape->getUserPointer();
    if (m) {
      delete m;
      shape->setUserPointer(nullptr);
      printf("%s(%p)\n", __FUNCTION__, shape);
    }
  }
}
void hideWorld(btDiscreteDynamicsWorld *dynamicsWorld) {
  for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--) {
    btCollisionObject *obj = dynamicsWorld->getCollisionObjectArray()[j];
    btRigidBody *body = btRigidBody::upcast(obj);
    btCollisionShape *shape = body->getCollisionShape();
    if (shape->isConvex()) {
      tMesh *m = (tMesh *)shape->getUserPointer();
      if (m) {
        delete m;
        shape->setUserPointer(nullptr);
        printf("%s(%p)\n", __FUNCTION__, shape);
      }
    }
  }
}
class Tri : public btTriangleCallback {
  tMesh *m;
  btIDebugDraw *d;
  float h;

public:
  Tri(tMesh *_m, btIDebugDraw *_d, float _h) {
    m = _m;
    d = _d;
    h = _h;
  }
  void processTriangle(btVector3 *triangle, int partId, int triangleIndex) {
    btVector3 &v0 = triangle[0];
    btVector3 &v1 = triangle[1];
    btVector3 &v2 = triangle[2];
    btVector3 tmp = (v1 - v0).cross(v2 - v0).normalize();
    /*if (v0.y() > h&&v1.y() > h&&v2.y() > h)*/ {
      m->v.push_back(v0.x());
      m->v.push_back(v0.y());
      m->v.push_back(v0.z());
      m->v.push_back(v1.x());
      m->v.push_back(v1.y());
      m->v.push_back(v1.z());
      m->v.push_back(v2.x());
      m->v.push_back(v2.y());
      m->v.push_back(v2.z());
      m->n.push_back(tmp.x());
      m->n.push_back(tmp.y());
      m->n.push_back(tmp.z());
      m->n.push_back(tmp.x());
      m->n.push_back(tmp.y());
      m->n.push_back(tmp.z());
      m->n.push_back(tmp.x());
      m->n.push_back(tmp.y());
      m->n.push_back(tmp.z());
    }
    /*printf("[%d,%d] %f, %f, %f\n", partId, triangleIndex, triangle->x(), triangle->y(), triangle->z());*/
  }
};
void showWorld(btDiscreteDynamicsWorld *dynamicsWorld, float scale) {
  /*gScale = scale;
  dynamicsWorld->debugDrawWorld();
  return;*/
  btScalar mtx[16];
  bool re = false;
  /*static btVector3 lastpan(0, 0, 0);
  btVector3 pan(position[0] * scale, 0, position[2] * scale);
  if ((lastpan - pan).length() > 5) {
    re = true;
    lastpan = pan;
  }*/
  for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--) {
    btCollisionObject *obj = dynamicsWorld->getCollisionObjectArray()[j];
    btRigidBody *body = btRigidBody::upcast(obj);
    btTransform trans;
    /*if (body && body->getMotionState()) {
      body->getMotionState()->getWorldTransform(trans);
    } else*/ {
      trans = obj->getWorldTransform();
    }
    btCollisionShape *shape = body->getCollisionShape();
#define PLANE_SIZE 200
    auto type = shape->getShapeType();
    if (type == STATIC_PLANE_PROXYTYPE) {
      static float mtx[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, scale};
      static float plane[18] = {-PLANE_SIZE, 0, -PLANE_SIZE, -PLANE_SIZE, 0, PLANE_SIZE, PLANE_SIZE, 0, PLANE_SIZE,
                                -PLANE_SIZE, 0, -PLANE_SIZE, PLANE_SIZE,  0, PLANE_SIZE, PLANE_SIZE, 0, -PLANE_SIZE};
      static float normal[18] = {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0};
      static btVector3 color(0.4, 0.6, 0.8);
      drawMesh(mtx, plane, normal, 0, 6, color);
    } else if (type == TERRAIN_SHAPE_PROXYTYPE) {
      tMesh *m = (tMesh *)shape->getUserPointer();
      if (m && re) {
        delete m;
        m = nullptr;
        shape->setUserPointer(nullptr);
      }
      if (!m) {
        m = new /*(btAlignedAlloc(sizeof(tMesh), 16))*/ tMesh();
        shape->setUserPointer(m);
        btHeightfieldTerrainShape *hs = (btHeightfieldTerrainShape *)shape;
        Tri callback(m, dynamicsWorld->getDebugDrawer(), -trans.getOrigin().y() - 0.0001);
        btVector3 aabbMin(-2500, -10, -2500);
        btVector3 aabbMax(2500, 10, 2500);
        // hs->processAllTriangles(&callback, aabbMin - trans.getOrigin() + pan, aabbMax - trans.getOrigin() + pan);
        hs->processAllTriangles(&callback, aabbMin, aabbMax);
        m->s = m->v.size() / 3;
      }
      trans.getOpenGLMatrix(mtx);
      mtx[12] -= position[0] * scale;
      // mtx[13] -= position[1] * scale;
      mtx[14] -= position[2] * scale;
      mtx[15] = scale;
      drawMesh(mtx, m->v.data(), m->n.data(), 0, m->s, btVector3(1, 1, 1));
    } else if (shape->isConvex()) {
      tMesh *m = (tMesh *)shape->getUserPointer();
      if (!m) {
        btConvexShape *cs = (btConvexShape *)shape;
        btShapeHull sc(cs);
        m = new /*(btAlignedAlloc(sizeof(tMesh), 16))*/ tMesh();
        sc.buildHull(cs->getMargin());
        shape->setUserPointer(m);
        const int ni = sc.numIndices();
        const int nv = sc.numVertices();
        const unsigned int *pi = sc.getIndexPointer();
        const btVector3 *pv = sc.getVertexPointer();
        
        bool isSphere = type == SPHERE_SHAPE_PROXYTYPE;
        // draw_setTransform(m);
        // draw_mesh(pv, sizeof(btVector3), pi, sc->numTriangles(), m);
        if (isSphere) {
          m->s = sc.numIndices();
          btSphereShape *sphere = (btSphereShape *)shape;
          for (int i = 0; i < sc.numVertices(); ++i) {
            auto v = pv[i];
            auto vv = v.normalized();
            m->v.push_back(vv.x()*sphere->getRadius());
            m->v.push_back(vv.y()*sphere->getRadius());
            m->v.push_back(vv.z()*sphere->getRadius());
            m->n.push_back(vv.x());
            m->n.push_back(vv.y());
            m->n.push_back(vv.z());
          }
          for (int i = 0; i < sc.numIndices(); ++i) {
            m->i.push_back(pi[i]);
          }
        } else {
          m->s = sc.numTriangles() * 3;
          for (int i = 0; i < sc.numTriangles(); ++i) {
            const btVector3 &v0 = pv[pi[i * 3 + 0]];
            const btVector3 &v1 = pv[pi[i * 3 + 1]];
            const btVector3 &v2 = pv[pi[i * 3 + 2]];
            // processTriangle(v3, 0, 0);
            m->v.push_back(v0.x());
            m->v.push_back(v0.y());
            m->v.push_back(v0.z());
            m->v.push_back(v1.x());
            m->v.push_back(v1.y());
            m->v.push_back(v1.z());
            m->v.push_back(v2.x());
            m->v.push_back(v2.y());
            m->v.push_back(v2.z());
            btVector3 tmp = (v1 - v0).cross(v2 - v0).normalize();
            m->n.push_back(tmp.x());
            m->n.push_back(tmp.y());
            m->n.push_back(tmp.z());
            m->n.push_back(tmp.x());
            m->n.push_back(tmp.y());
            m->n.push_back(tmp.z());
            m->n.push_back(tmp.x());
            m->n.push_back(tmp.y());
            m->n.push_back(tmp.z());
          }
        }
        // draw_done_face();
      }
      trans.getOpenGLMatrix(mtx);
      mtx[12] -= position[0] * scale;
      // mtx[13] -= position[1] * scale;
      mtx[14] -= position[2] * scale;
      mtx[15] = scale;
      drawMesh(mtx, m->v.data(), m->n.data(), m->i.size()>0?m->i.data():0, m->s, btVector3(1, 1, 1));
    }
  }
}