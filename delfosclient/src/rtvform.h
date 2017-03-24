#ifndef RTVFORM_H
#define RTVFORM_H

// Qt includes
#include <QWidget>
#include <wcore/utils/unit_test/tester/RTVTester.h>

// Project includes
#include <src/commonparameters.h>

namespace Ui {
class RTVForm;
}

class RTVForm : public QWidget
{
  Q_OBJECT
private:
  Ui::RTVForm *ui;

  /**
   * @brief log_iface
   */
  delfosErrorHandler _weh;

  /**
   * @brief _rm_client client for rm
   */
  RTVTester* _rtv_client;

  /**
   * @brief _session_guid Stores the last session_guid set
   */
  string _session_guid;

  /**
   * @brief _test_image test image
   */
  char* _test_image;
  size_t _test_image_size;

  /**
   * @brief init_test_image Initialises test image
   */
  void init_test_image();

public:
  explicit RTVForm(QWidget *parent = 0);
  ~RTVForm();

private slots:

  /**
   * Clicked image
   */
  void on_setImageButton_clicked();
  void on_getImageButton_clicked();
};

#endif // RTVFORM_H
