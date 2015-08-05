/*
 * Cloud-based Object Recognition Engine (CORE)
 *
 */

#include <core/learning/learn_svm_covariance_model.h>

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

int 
getCategories (const std::string file_name, std::vector<std::string> &categories)
{
  std::string line;
  std::ifstream fs;

  fs.open (file_name.c_str (), std::ios::in);
  if (!fs.is_open () || fs.fail ())
  {
      CORE_ERROR ("Could not open file '%s'! Error : %s\n", file_name.c_str (), strerror (errno));
      fs.close ();
      return (-1);
  }

  while (!fs.eof ())
  {
    getline (fs, line);
    if (line == "")
      continue;
    categories.push_back (line);
  } 

  fs.close ();

  return (0);
}

int 
getCovariances (const std::string dir_name, std::vector<std::string> &covariances)
{
  DIR* dp;
  struct dirent* dirp;

  if ((dp = opendir (dir_name.c_str ())) == NULL) 
  {
    CORE_ERROR ("Could not open directory '%s'! Error : %s\n", dir_name.c_str (), strerror (errno));
    return (-1);
  }

  while ((dirp = readdir (dp)) != NULL) 
  { 
    if ((strcmp (dirp->d_name, ".") == 0) || (strcmp (dirp->d_name, "..") == 0))
      continue;
    covariances.push_back (dir_name + "/" + dirp->d_name);
  }

  closedir(dp);

  return (0);
}

int
learnModel (double gamma, const std::string category_file_list)
{
  std::vector<std::string> categories;
  std::vector<std::string> covariances;
  std::vector<std::vector<svm_node> > data;
  std::vector<int> labels;
  struct svm_parameter param;   
  struct svm_problem prob;      
  struct svm_model* model;
  struct svm_node* x_space;     
  int ret_val, label = 1;

  if ((ret_val = getCategories (category_file_list, categories)) < 0)
    return (-1);
  for (std::vector<std::string>::iterator it = categories.begin(); it != categories.end (); ++it)
  {
    if ((ret_val = getCovariances(*it, covariances)) < 0)
      continue;
    while (!covariances.empty ())
    {
      std::ifstream fs;
      std::vector<svm_node> nodes;
      svm_node node;
      double value;
      int index = 1;

      std::string covariance = covariances.back ();
      covariances.pop_back ();
      fs.open (covariance.c_str (), std::ios::in);
      if (!fs.is_open () || fs.fail ())
      {
        CORE_ERROR ("Could not open file '%s'! Error : %s\n", covariance.c_str (), strerror (errno));
        fs.close ();
        continue;
      }
      // Fill the feature vector
      while (fs >> value)
      {
        node.index = index;
        node.value = value;
        nodes.push_back (node);
        ++index;
      }
      // Terminate the feature vector
      node.index = -1;
      nodes.push_back (node);
      data.push_back (nodes);     
      labels.push_back (label); 
      fs.close();
    }
    // Update the label when changing category
    ++label;
  }
  
  // Make sure we have data
  if (data.empty ())
  {
    CORE_ERROR ("No training data learned\n");
    return (-1);  
  }
  
  prob.l = data.size ();
  prob.y = Malloc (double, prob.l);
  prob.x = Malloc (struct svm_node* , prob.l);
  x_space = Malloc (struct svm_node, data.size () * data[0].size ());
  
  int j = 0;
  for (int i = 0; i < prob.l; ++i)
  {
    prob.y[i] = *(labels.begin () + i);
    prob.x[i] = &x_space[j];
    for (std::vector<svm_node>::iterator it = data[i].begin (); it != data[i].end (); ++it)
    {
      x_space[j].index = it->index;
      x_space[j].value = it->value;
      ++j;
    } 
  }

  param.svm_type = C_SVC;
  param.kernel_type = RBF;
  param.gamma = gamma; 
  param.degree = 3;
  param.coef0 = 0;
  param.nu = 0.5;
  param.cache_size = 100;
  param.C = 1;
  param.eps = 1e-3;
  param.p = 0.1;
  param.shrinking = 1;
  param.probability = 0;
  param.nr_weight = 0;
  param.weight_label = NULL;
  param.weight = NULL;

  // Check whether the parameters are within a feasible range of the problem
  const char* error_msg = svm_check_parameter (&prob, &param);
  if (error_msg)
  {
    CORE_ERROR ("Error checking SVM parameters: %s\n", error_msg);
    return (-1);
  }

  model = svm_train (&prob, &param);
  if (svm_save_model ("model.txt", model))
  {
    CORE_ERROR ("Save SVM model failed\n");
    return (-1);
  }

  svm_free_and_destroy_model (&model);
  svm_destroy_param (&param);
  free (prob.y);
  free (prob.x);
  free (x_space);

  return (0);
}

void
printHelp (int, char** argv)
{
  CORE_INFO ("Syntax is: %s gamma (learning parameter) categories (directory list of covariances)\n", argv[0]);
}

int 
main (int argc, char** argv)
{
  if (argc < 2)
  {
    printHelp (argc, argv);
    return (-1);
  } 

  double gamma = static_cast<double> (atof (argv[1]));
  std::string category_file_list = argv[2];

  learnModel (gamma, category_file_list);
  
  return (0);
}