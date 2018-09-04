#ifndef NODE_H_
#define NODE_H_
#include "vmath.h"
#include <list>
#define RAD2DEG(x) (((x) * 180.0) / M_PI)


template<class T>
class Node {
private:
	Matrix4<T> _matrix;
	Node * parent;
	T a_x;
	T a_y;
	T a_z;
	std::list<Node *> childs;

public:
	struct Freedom {
		T min;
		T max;
	};

	Freedom * freedom_x_angle;
	Freedom * freedom_y_angle;
	Freedom * freedom_z_angle;

	Node():
			parent(NULL),
			a_x(0.0),
			a_y(0.0),
			a_z(0.0),
			freedom_x_angle(NULL),
			freedom_y_angle(NULL),
			freedom_z_angle(NULL) {
	};

	~Node() {
		if (parent) {
			parent->childs.remove(this);
		}
	}

	void set_parent(Node * _parent) {
		if(_parent == parent) {
			return;
		}

		parent = _parent;
		if (parent) {
			parent->childs.push_back(this);
		}
	}

	Matrix4<T> matrix() const {
		std::cout << "matrix this:" << this << " parent:" << parent << std::endl;
		return parent ? parent->matrix() * _matrix : _matrix; 
	}

	Vector3<T> to_world(const Vector3<T> & pos) const {
		Matrix4<T> mat(matrix());
		std::cout << "to_world this:" << this << " pos:" << pos << std::endl;
		std::cout << mat << std::endl;
		return (mat * Vector4<T>(pos, 1.0)).xyz();
	}

	Vector3<T> from_world(const Vector3<T> & pos) const {
		// std::cout << "from_world 1. this:" << this << " pos:" << pos << std::endl;

		Matrix4<T> mat(matrix());
		// std::cout << "from_world 2. mat:" << mat << std::endl;

		Vector3<T> l_pos(pos - mat.getTranslation());

		// std::cout << "from_world 3. l_pos:" << l_pos << std::endl;
		Vector3<T> res =  mat.transpose() * l_pos; 
		// std::cout << "from_world 4. res:" << res << std::endl;
		return res;
	}

	void set_pos(const Vector3<T> & pos) {
		_matrix.setTranslation(pos);
	}

	Vector3<T> get_pos() const {
		return _matrix.getTranslation();
	}

	void set_g_pos(const Vector3<T> & g_pos) {
		return set_pos(from_world(g_pos));
	}

	Vector3<T> get_g_pos() const {
		return matrix().getTranslation();
	}

	T get_angle_x() const { return a_x;}

	void set_angle_x(T angle_deg) {
		//std::cout << "set_angle_x angle_deg:" << angle_deg << std::endl;

		if (a_x != angle_deg) {
			a_x = angle_deg;
			_matrix.setRotation(Matrix3<T>::createRotationAroundAxis(a_x, a_y, a_z));
		}
	}

	T get_angle_y() const { return a_y;}

	void set_angle_y(T angle_deg) {
		if (a_y != angle_deg) {
			a_y = angle_deg;
			_matrix.setRotation(Matrix3<T>::createRotationAroundAxis(a_x, a_y, a_z));
		}
	}

	T get_angle_z() const { return a_z;}

	void set_angle_z(T angle_deg) {
		if (a_z != angle_deg) {
			a_z = angle_deg;
			_matrix.setRotation(Matrix3<T>::createRotationAroundAxis(a_x, a_y, a_z));
		}
	}

	static T get_proj_angle(
		const Vector3<T> & axis,
		const Vector3<T> & up,
		const Vector3<T> & vec) {
		// std::cout << "get_proj_angle 1. axis:" << axis << " up:" << up << " vec:" << vec << std::endl;
		T res = atan2(
			up.dotProduct(vec), axis.dotProduct(vec));
		// std::cout << "get_proj_angle 2. res:" << res << std::endl;
		return res;
	}
};

typedef Node<float> NodeF;

#endif