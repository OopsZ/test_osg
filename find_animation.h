#pragma once

class FindAnimationVisitor : public osg::NodeVisitor 
{ 
public: 
    FindAnimationVisitor(): 
      osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)  
      { 
          //apc=NULL; 
          setTraversalMask(0xffffffff); 
          setNodeMaskOverride(0xffffffff); 
      } 


      //��������� �������� 
      typedef std::vector< osg::ref_ptr<osg::AnimationPathCallback> > NodeList; 
      NodeList _foundNodes; 
      //� �������������� �������, ���������� ��� ����� �������� 
      typedef std::vector< osg::ref_ptr<osg::Transform> > TransformList; 
      TransformList _foundTransforms; 

      virtual void apply(osg::Transform& node); 
      virtual void apply(osg::Group& group); 
      /* 
      virtual void apply(osg::MatrixTransform& node); 
      virtual void apply(osg::AnimationPathCallback& node); 
      */ 


private: 
    //���� �������� 
    //osg::ref_ptr<osg::AnimationPathCallback> apc; 
}; 




void FindAnimationVisitor::apply(osg::Transform& node) 
{ 
    //apc = dynamic_cast<osg::AnimationPathCallback*>(node.getUpdateCallback()); 
    osg::ref_ptr<osg::AnimationPathCallback> test = dynamic_cast<osg::AnimationPathCallback*>(node.getUpdateCallback()); 
    if (test.get()) 
    { 
        //std::cout << "I find animation!!!!"; 
        //���������� �������� 
        _foundNodes.push_back (dynamic_cast<osg::AnimationPathCallback*>(node.getUpdateCallback())); 
        //� ������ � �������� ��� ����������� 
        _foundTransforms.push_back (&node); 
    } 
    traverse(node); 
} 


void FindAnimationVisitor::apply(osg::Group& group)  
{ 
    traverse(group);  
}  