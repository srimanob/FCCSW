//
//  Surface.cxx
//  
//
//  Created by Julia Hrdinka on 25/11/14.
//
//

#include "RecoGeometry/Surface.h"

double Reco::Surface::s_onSurfaceTolerance               = 10e-5; // 0.1 * micron
double Reco::Surface::s_zeroLimit                        = 10e-12;
Alg::Point3D    Reco::Surface::s_origin;
Alg::Transform3D Reco::Surface::s_idTransform;


Reco::Surface::Surface() :
m_center    (0),
m_normal    (0),
m_transform (0),
m_materialmap  (0)
{}

Reco::Surface::Surface(std::shared_ptr<const Alg::Transform3D> transf) :
m_center    (0),
m_normal    (0),
m_transform (transf)
{}

Reco::Surface::Surface(TGeoNode* node) :
m_center    (0),
m_normal    (0),
m_materialmap(0)
{
    //get Transformation
    const Double_t* rotation    = (node->GetMatrix()->GetRotationMatrix());
    const Double_t* translation = (node->GetMatrix()->GetTranslation());
    
    m_transform = std::make_shared<const Alg::Transform3D> (Alg::Transform3D(
                                       rotation[0], rotation[1], rotation[2], translation[0],
                                       rotation[3], rotation[4], rotation[5], translation[1],
                                       rotation[6], rotation[7], rotation[8], translation[2]));
}

//constructor to set material
Reco::Surface::Surface(TGeoNode* node, MaterialMap* materialmap) :
m_center(0),
m_normal(0),
m_materialmap(materialmap)
{
    //get Transformation
    const Double_t* rotation    = (node->GetMatrix()->GetRotationMatrix());
    const Double_t* translation = (node->GetMatrix()->GetTranslation());
    
    m_transform = std::make_shared<const Alg::Transform3D> (Alg::Transform3D(
                                    rotation[0], rotation[1], rotation[2], translation[0],
                                    rotation[3], rotation[4], rotation[5], translation[1],
                                    rotation[6], rotation[7], rotation[8], translation[2]));
}

Reco::Surface::Surface(MaterialMap* materialmap, std::shared_ptr<const Alg::Transform3D> transf) :
m_center(0),
m_normal(0),
m_transform (transf),
m_materialmap(materialmap)
{}

Reco::Surface::Surface(const Reco::Surface& surface) :
m_center    (0),
m_normal    (0),
m_transform (0),
m_materialmap  (0)
{

    m_transform  = std::make_shared<const Alg::Transform3D>(surface.transform());
    m_materialmap= materialmap()->clone();
}

Reco::Surface::~Surface()
{
    delete m_center;
    delete m_normal;
    m_materialmap = 0;
}

Reco::Surface& Reco::Surface::operator=(const Reco::Surface& surface)
{
    if (this!=&surface){
        delete m_center; m_center = 0;
        delete m_normal; m_normal = 0;
        
        m_transform = surface.m_transform;
        m_materialmap = surface.m_materialmap;
    }
    return (*this);
}

const Alg::Transform3D& Reco::Surface::transform() const
{
    if(m_transform) return (*m_transform);
    return s_idTransform;
}

void Reco::Surface::setTransform(std::shared_ptr<const Alg::Transform3D> transf)
{
    m_transform = transf;
}

const Alg::Point3D& Reco::Surface::center() const
{
    if (!m_transform && !m_center) return (s_origin);
    else if (!m_center) m_center = new Alg::Point3D(m_transform->Translation().Vect());
    return (*m_center);
}

const Alg::Vector3D&  Reco::Surface::normal() const
{
    if (m_transform && !m_normal) {
        Alg::Vector3D lx(0.,0.,0.);
        Alg::Vector3D ly(0.,0.,0.);
        m_normal = new Alg::Vector3D;
        m_transform->Rotation().GetComponents<Alg::Vector3D>(lx,ly,*m_normal);
    }
    return (*m_normal);
}

const Alg::Vector3D* Reco::Surface::normal(const Alg::Point2D&) const
{
    return (new Alg::Vector3D(normal()));
}

const Reco::MaterialMap* Reco::Surface::materialmap() const
{
    return (m_materialmap);
}

double Reco::Surface::pathlength(const Alg::Vector3D& dir) const
{
    Alg::Vector3D d     = dir.Unit();
    Alg::Vector3D n     = normal().Unit();
//o    std::cout << "normal: " << n << std::endl;
    double pathlength   = 0.;
    double phi = acos(d.Dot(n));
    if (phi!=0.5*M_PI) {
        double theta = (0.5*M_PI)-phi;
        pathlength = 2.*(halfThickness()/(sin(theta)));
//        std::cout << "Phi: " << phi << " Theta: " << theta << " halfthickness: " << halfThickness() << " dir: " << dir << " phi: " << phi << " normal: " << n << std::endl;
        if (pathlength<0.) {
  //          std::cout << "negativ! Theta: " << theta << " halfthickness: " << halfThickness() << " dir: " << dir << " phi: " << phi << " normal: " << n << std::endl;
        }
    }
    return(pathlength);
}


