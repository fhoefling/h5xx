#ifndef H5XX_ITERATOR_HPP
#define H5XX_ITERATOR_HPP

#include <h5xx/group.hpp>
#include <h5xx/dataset.hpp>

namespace h5xx
{

template <typename T>
class iterator
{

    private:

        /** reference to container group **/
	group& container_group;

        /** id of container group **/
        hid_t container_group_id;

        /** container of names of elements in group **/
        std::vector<std::string> names_of_elements;

        /** internal iterator over element names **/
        std::vector<std::string>::iterator internal_iter;

    public:

        iterator(group&);
        iterator(const iterator&);
        ~iterator();

	/** operators for iterator arithmetic **/
	int operator+(const iterator&);
	int operator-(const iterator&);

	//iterator operator+(int);
        //iterator operator-(int);

	iterator operator+=(int);
	iterator operator-=(int);

        /** increment and decrement operators move to next element **/
        iterator& operator++();
        iterator& operator++(int);

        iterator& operator--();
        iterator& operator--(int);

        /** returns h5xx-object **/
        T operator*();

        /** comparison operators **/
        /** determined on basis of hdf5 id **/
        bool operator==(const iterator&);
        bool operator!=(const iterator&);

        /** begin() and end() methods **/
        iterator begin();
        iterator end();

        void print_names();

}; // ! iterator

// TODO: is there a better way using more h5xx than hdf5?
template <typename T>
herr_t find_names_of_type(hid_t g_id, const char* name, const H5L_info_t *info, void *op_data);


template <typename T>
inline iterator<T>::iterator(group& group) : container_group(group)
{
    // assuming container_group is legit h5xx::group object and open
    container_group_id = container_group.hid();
    hsize_t stop_idx = 0; // TODO: does this work in the general case ??
    H5Literate(container_group_id, H5_INDEX_NAME, H5_ITER_NATIVE, &stop_idx, find_names_of_type<T>, &names_of_elements);
    internal_iter = names_of_elements.begin(); //TODO: this is only a foreward iterator. should there be backward too??
}


template <typename T>
inline iterator<T>::iterator(const iterator& other) : container_group(other.container_group)
{
    container_group_id = other.container_group_id;
    names_of_elements = other.names_of_elements;
    internal_iter = names_of_elements.begin() + (other.internal_iter - other.names_of_elements.begin());
}


template <typename T>
inline iterator<T>::~iterator(){}


template <typename T>
inline bool iterator<T>::operator==(const iterator& other)
{
    if(internal_iter == other.internal_iter // TODO: sollte nur das referenzierte objekt betrachtet werden?
       && container_group_id == other.container_group_id)
        return true;
    else
        return false;
}


template <typename T>
inline bool iterator<T>::operator!=(const iterator& other)
{
    if(internal_iter == other.internal_iter
       && container_group_id == other.container_group_id)
        return false;
    else
        return true;
}


template <typename T>
inline int iterator<T>::operator+(const iterator& other)
{
    if(container_group_id == other.container_group_id)
    {
        int internal_offset = internal_iter - names_of_elements.begin();
        int other_offset = other.internal_iter - other.names_of_elements.begin();
        return(internal_offset + other_offset);
    }
    else
    {
        throw std::domain_error("Container groups are not identical");
    }
}


template <typename T>
inline int iterator<T>::operator-(const iterator& other)
{
    if(container_group_id == other.container_group_id)
    {
        int internal_offset = internal_iter - names_of_elements.begin();
        int other_offset = other.internal_iter - other.names_of_elements.begin();
        return(internal_offset - other_offset);
    }
    else
    {
        throw std::domain_error("Container groups are not identical");
    }
}


template <typename T>
inline iterator<T>& iterator<T>::operator++()
{
    internal_iter++; // TODO: scope checking??
    return(*this);
}


template <typename T>
inline iterator<T>& iterator<T>::operator++(int)
{
    internal_iter++;
    return(*this);
}


template <typename T>
inline iterator<T>& iterator<T>::operator--()
{
    internal_iter--;
    return(*this);
}


template <typename T>
inline iterator<T>& iterator<T>::operator--(int)
{
    internal_iter--;
    return(*this);
}


// TODO: sind diese templates überhaupt sinnvoll?? was passiert bei anderen Werten der Template-Parameter ??
template <>
herr_t find_names_of_type<h5xx::group>(hid_t g_id, const char* name, const H5L_info_t *info, void *op_data)
{
    H5O_info_t obj_info;
    herr_t retval = H5Oget_info_by_name(g_id, name, &obj_info, H5P_DEFAULT);
    if(obj_info.type == H5O_TYPE_GROUP)
    {
        // TODO: how about dynamic_cast/static_cast ?
        std::vector<std::string> *vec_ptr = (std::vector<std::string> *) op_data;
        (*vec_ptr).push_back((std::string) name);
    }

    return(retval);
}


template <>
herr_t find_names_of_type<h5xx::dataset>(hid_t g_id, const char* name, const H5L_info_t *info, void *op_data)
{
    H5O_info_t obj_info;
    herr_t retval = H5Oget_info_by_name(g_id, name, &obj_info, H5P_DEFAULT);
    if(obj_info.type == H5O_TYPE_DATASET)
    {
        std::vector<std::string> *vec_ptr = (std::vector<std::string> *) op_data;
        (*vec_ptr).push_back((std::string) name);
    }

    return(retval);
}


template <>
h5xx::group iterator<h5xx::group>::operator*()
{
    h5xx::group group(container_group, *internal_iter);
    std::cout << *internal_iter << std::endl;
    return(h5xx::move(group));
}


template <>
h5xx::dataset iterator<h5xx::dataset>::operator*()
{
    h5xx::dataset dset(container_group, *internal_iter);
    return(h5xx::move(dset));
}


template <typename T>
iterator<T> iterator<T>::begin()
{
    iterator<T> iter_begin(*this);
    iter_begin.internal_iter = iter_begin.names_of_elements.begin();
    return(iter_begin);
}


template <typename T>
iterator<T> iterator<T>::end()
{
    iterator<T> iter_end(*this);
    iter_end.internal_iter = iter_end.names_of_elements.end();
    return(iter_end);
}


template <typename T>
void iterator<T>::print_names()
{
    for(std::vector<std::string>::iterator it = names_of_elements.begin(); it != names_of_elements.end(); it++)
    {
        std::cout << *it << std::endl;
    }
}
} // ! h5xx

#endif // ! H5XX_ITERATOR
