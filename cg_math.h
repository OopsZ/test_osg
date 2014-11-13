namespace cg
{
    template<typename T, typename D>
    __forceinline D lerp01( const T x, const D & y0, const D & y1 )
    {
        return y0 + (y1 - y0) * x;
    }

    __forceinline double min(double a, double b) { return a < b ? a : b; }
    __forceinline double max(double a, double b) { return a > b ? a : b; }

    __forceinline float min(float a, float b) { return a < b ? a : b; }
    __forceinline float max(float a, float b) { return a > b ? a : b; }

    template<typename T> 
    __forceinline T sqr( const T val )
    {
        return val * val;
    }

    template <class T, class D = T>
    struct Lerp
    {
        typedef
            typename meta::_if< meta::_is_integral< D >, T, D >::type
            tform_type;

        // �������� �������������� [x0,x1] --> [y0,y1]
        __forceinline Lerp(T x0, T x1, D y0, D y1)
            : K_(cg::eq_zero(x1 - x0) ? tform_type() * T(0.) : ( y1 - y0 ) * ( T(1) / (x1 - x0) ))
            , D_( y0 - K_ * x0 )
        {}

        __forceinline D operator() (T x) const {
            return ( D ) ( K_*x + D_ );
        }

        __forceinline tform_type const& K() const {
            return K_ ;
        }

        __forceinline tform_type const& B() const {
            return D_ ;
        }

    private:
        tform_type K_;
        tform_type D_;
    };

    template <class T, class D = T>
    struct Clamp
    {
        __forceinline Clamp(T x0, T x1, D y0, D y1)
            : x0(x0), x1(x1), y0(y0), y1(y1)
            , l_( x0, x1, y0, y1 )
        {}

        __forceinline D operator () (T x) const {
            return
                x <= x0 ? y0 :
                x >= x1 ? y1 :
                l_(x);
        }

    private:
        T   x0, x1;
        D   y0, y1;
        Lerp<T, D> l_;
    };

    template <class T> __forceinline  Clamp<T> clamp(T x0, T x1, T y0, T y1)
    {
        return Clamp<T>(x0, x1, y0, y1);
    }

    template <class type>   type    epsilon         () { return type(); }
    template <> inline      double  epsilon<double> () { return 1e-10; }
    template <> inline      float   epsilon<float > () { return 1e-6f; }

    template <class type>   bool eq (type const& lhs, type const& rhs) { return lhs == rhs; }
    inline                  bool eq (float       lhs, float       rhs) { return fabs(rhs - lhs) < epsilon<double>(); }
    inline                  bool eq (double      lhs, double      rhs) { return fabs(rhs - lhs) < epsilon<float >(); }

    template <class type>   bool eq_zero(type const& value) { return eq(value, type()); }
    // fuzzy equality to 0
    __forceinline bool eq_zero (float  a, float  eps = epsilon< float >( ) ) { return abs(a) <= eps; }
    __forceinline bool eq_zero (double a, double eps = epsilon< double >( )) { return abs(a) <= eps; }
    __forceinline bool eq_zero (int a, double eps = epsilon< int >( )) { return abs(double(a)) <= eps; }

    // fuzzy equal
    __forceinline bool eq (float a,  float b,  float  eps = epsilon< float >( )) { return abs(a - b) <= eps; }
    __forceinline bool eq (float a,  int   b,  float  eps = epsilon< float >( )) { return abs(a - b) <= eps; }
    __forceinline bool eq (double a,  double b,  double  eps = epsilon< double >( )) { return abs(a - b) <= eps; }
    __forceinline bool eq (double a,  int    b,  double  eps = epsilon< double >( )) { return abs(a - b) <= eps; }
    __forceinline bool eq (float  a,  int    b,  double  eps = epsilon< double >( )) { return abs((double)a - b) <= eps; }

    //
    template <class V>
    __forceinline typename V::value_type luminance_crt( V const & col )
    {
        return V::value_type(0.299f * col.x() + 0.587f * col.y() + 0.114f * col.z());
    }

    //
    template <typename V>
    __forceinline typename V::value_type luminance_lcd( V const & col )
    {
        return V::value_type(0.2127f * col.x() + 0.7152f * col.y() + 0.0722f * col.z());
    }


    template < class T >
    struct Bound
    {
        Bound( T min_val, T max_val )
            : min_val_( min_val )
            , max_val_( max_val )
        {}

        T operator( ) ( T val ) const
        {
            return val > max_val_ ? max_val_ : val < min_val_ ? min_val_ : val ;
        }

    private:
        T min_val_;
        T max_val_;
    };

    template<class T> __forceinline
        Bound<T> bound( T vmin, T vmax )
    {
        return Bound<T>( vmin, vmax );
    }

    template<class T> __forceinline
        T bound(T x, T vmin, T vmax)
    {
        return x < vmin ? vmin : x > vmax ? vmax : x;
    }


};