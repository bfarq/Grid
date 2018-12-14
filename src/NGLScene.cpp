#include <QMouseEvent>
#include <QGuiApplication>
#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleVAO.h>
#include <ngl/ShaderLib.h>
#include <ngl/Random.h>
#include <ngl/NGLStream.h>
#include <iostream>
#include <ngl/fmt/format.h>
#include <ngl/NGLMessage.h>

NGLScene::NGLScene(size_t _numMeshes)
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  setTitle(fmt::format("Blank NGL Num Mesheds {0}",_numMeshes).c_str());
  m_meshes.resize(_numMeshes);
  createMeshes();
  m_collection.resize(4);
  updateCollection();
  startTimer(10);
}

void NGLScene::addMesh(MeshType _m)
{
    ngl::Random *rng = ngl::Random::instance();
    MeshData m;
    m.pos=rng->getRandomPoint(40,0,40);
    m.rot.m_y=rng->randomPositiveNumber(360.0f);
    m.scale.set(1.0f,1.0f,1.0f);
    m.colour=rng->getRandomColour4();
    m.type=_m;
    m_meshes.push_back(m);
}

void NGLScene::createMeshes()
{

    ngl::Random *rng =ngl::Random::instance();
    for(auto &m : m_meshes)
    {
        m.pos=rng->getRandomPoint(40,0,40);
        m.rot.m_y=rng->randomPositiveNumber(360.0f);
        m.scale.set(1.0f,1.0f,1.0f);
        m.colour=rng->getRandomColour4();
        int type = static_cast<int>(rng->randomPositiveNumber(4));
        switch(type)
        {
            case 0 : m.type=MeshType::CUBE; break;
            case 1 : m.type=MeshType::TEAPOT; break;
            case 2 : m.type=MeshType::TROLL; break;
            case 3 : m.type=MeshType::SPHERE; break;
        }
    }
}

NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}



void NGLScene::resizeGL(int _w , int _h)
{
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
  m_project=ngl::perspective(45.0f, static_cast<float>(_w)/_h,0.5f,200.0f);
}

constexpr auto *ColourShader("ColourShader");
constexpr auto *LineShader("LineShader");

void NGLScene::initializeGL()
{
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::instance();
  glClearColor(0.8f, 1.0f, 1.0f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  ngl::VAOPrimitives::instance()->createSphere("sphere",1.0f,40);
  ngl::ShaderLib *shader = ngl::ShaderLib::instance();
  shader->loadShader(ColourShader, "shaders/ColourVertex.glsl", "shaders/ColourFragment.glsl");
  shader->loadShader(LineShader, "shaders/ColourLineVertex.glsl", "shaders/ColourLineFragment.glsl");

  m_view=ngl::lookAt({0.0f,20.0f,20.0f},ngl::Vec3::zero(),ngl::Vec3::up());
  m_vao=ngl::VAOFactory::createVAO(ngl::simpleVAO, GL_LINES);
  //shader->use(ColourShader);
  //shader->setUniform("MVP",m_project*m_view);
}

void NGLScene::loadMatrixToShader(const ngl::Mat4 &_tx, const ngl::Vec4 &_colour) //FIX
{
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->use(ColourShader);
    shader->setUniform("MVP", m_project* m_view* _tx);
    //std::cout << "Load Matrix \n";
    //std::cout<< m_view << "\n" << m_project << "\n" << _tx << "\n" << m_project*m_view*_tx << "\n";
    //std::cout << "********************************** \n";
    shader->setUniform("vertColour", _colour);
}
void NGLScene::loadMatrixToLineShader(const ngl::Mat4 &_tx)
{
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->use(LineShader);
    shader->setUniform("MVP", m_project* m_view* _tx);
}


void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_win.width,m_win.height);
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  ngl::Mat4 mouseRotation;
  rotX.rotateX(m_win.spinXFace);
  rotY.rotateY(m_win.spinYFace);
  mouseRotation=rotY*rotX;



  ngl::Transformation tx;
  auto *prim=ngl::VAOPrimitives::instance();
  for(auto m : m_meshes)
  {
      tx.setPosition(m.pos);
      tx.setRotation(m.rot);
      tx.setScale(m.scale);
      loadMatrixToShader(mouseRotation*tx.getMatrix(), m.colour);

      switch(m.type)
      {
        case MeshType:: TEAPOT : prim->draw("teapot"); break;
        case MeshType:: CUBE : prim->draw("cube"); break;
        case MeshType:: TROLL : prim->draw("troll"); break;
        case MeshType:: SPHERE : prim->draw("sphere"); break;
      }
  }
  if(m_drawLines==true)
  {
    loadMatrixToLineShader(mouseRotation);
    drawLines(mouseRotation);

    //std::cout<<"mouseRotation.="<<mouseRotation.getBackVector()<<'\n';
  }
};

  /*tx.setScale(0.2f,0.2f,0.2f);
  for(float z=-20.0f; z<20.0f; z+=0.5f)
  {
    for(float x=-20.0f; x<20.0f; x+=0.5f)
    {
      ngl::Vec4 colour(z,z,x,1.0f);
      //colour.normalize();
      tx.setPosition(x,0.0f,z);
      //tx.setRotation(45,22,18);
      loadMatrixToShader(mouseRotation*tx.getMatrix(), colour);
      ngl::VAOPrimitives::instance()->draw("teapot");
    }
  }*/




//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  case Qt::Key_Space :
      m_win.spinXFace=0;
      m_win.spinYFace=0;
      m_modelPos.set(ngl::Vec3::zero());

  break;
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
  case Qt::Key_L : m_drawLines^=true; break;
  case Qt::Key_C : if (m_meshes.size()<=0) m_meshes.resize(100); createMeshes(); break;

  case Qt::Key_1 : addMesh(MeshType::TEAPOT); break;
  case Qt::Key_2 : addMesh(MeshType::CUBE); break;
  case Qt::Key_3 : addMesh(MeshType::SPHERE); break;
  case Qt::Key_4 : addMesh(MeshType::TROLL); break;


  default : break;
  }
  // finally update the GLWindow and re-draw

    update();
}

void NGLScene::updateCollection()
{
    for(auto &c : m_collection)
        c.clear();

    for(auto &m : m_meshes)
    {
        switch(m.type)
        {
            case MeshType::TEAPOT : m_collection[0].push_back(&m); break;
            case MeshType::CUBE   : m_collection[1].push_back(&m); break;
            case MeshType::SPHERE : m_collection[2].push_back(&m); break;
            case MeshType::TROLL  : m_collection[3].push_back(&m); break;
       }
    }
}


void NGLScene::drawLines(const ngl::Mat4 &_tx)
{
    std::vector <Vertex> line;
    std::array<ngl::Vec4,4> colours={{
         ngl::Vec4(1.0f,0.0f, 0.0f), //red teapot
         ngl::Vec4(0.0f,1.0f, 0.0f), //green cube
         ngl::Vec4(0.0f,0.0f, 1.0f), //blue sphere
         ngl::Vec4(1.0f,1.0f, 1.0f), //white troll

    }};

    Vertex a,b;

    for(size_t i=0; i<m_collection.size(); ++i)
    {
        a.colour=colours[i];
        b.colour=colours[i];
        auto size=m_collection[i].size();
        for(size_t original =0; original<size; ++original)
        {
            for (size_t current =0; current<size; ++current)
            {
                if (original==current) continue;
                a.pos.set(m_collection[i][original]->pos);
                b.pos.set(m_collection[i][current]->pos);
                line.push_back(a);
                line.push_back(b);
            }
        }
    }


    std::cout<<"num teapots "<<m_collection[0].size()<<" num meshes "<<m_meshes.size()<<'\n';

    m_vao->bind();
    m_vao->setData(ngl::SimpleVAO::VertexData(line.size()*sizeof(Vertex), line[0].pos.m_x));
    m_vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(Vertex),0);
    m_vao->setVertexAttributePointer(1,4,GL_FLOAT,sizeof(Vertex),3);
    m_vao->setNumIndices(line.size());
    m_vao->draw();
    m_vao->unbind();

}
void NGLScene::prune()
{
    for(auto it = m_meshes.begin(); it !=m_meshes.end(); )
    {
        if(it->distance < 0.05f )
        {
            it = m_meshes.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void NGLScene::timerEvent(QTimerEvent *_event)
{
    //ngl::NGLMessage::addMessage("timer",Colours:;YELLOW,TimeFormat:;TIMED);
    updateCollection();
    for (size_t i=0;i<m_collection.size(); ++i )
    {
        ngl::Vec3 center;
        for(auto m : m_collection[i])
            center+=m->pos;
        center /=m_collection[i].size();
        //std::cout<<"Center is " << center<<'\n';
        for(auto &m : m_collection[i])
        {
            m->dir=center-m->pos;
            m->distance=m->dir.length();
            m->dir.normalize();
            m->pos+=m->dir *0.1f;
        }
    }
    prune();
    update();
}
