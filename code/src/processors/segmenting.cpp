#include "segmenting.h"

Segmenting::Segmenting(QObject *parent)
  : Processor(parent), m_adapt(true), m_background(LIGHT)
{
  m_threshold = 0;
}

Segmenting::~Segmenting()
{
}

void Segmenting::process()
{
  if(input_image.empty()) return;

  Scalar m = mean(input_image);
  m_threshold = cvRound(m[0]);

  if(m_adapt) {
    qDebug("Threshold before: %d", m_threshold);
    adaptThreshold();
    qDebug("Threshold after: %d", m_threshold);
  }


  if(m_background == DARK)
    output_image = input_image >= m_threshold;
  else
    output_image = input_image < m_threshold;

  emit updated();
}

void Segmenting::adaptThreshold()
{
  int old_threshold;
  do {
    old_threshold = m_threshold;
    Mat above = input_image >= m_threshold;
    Mat below = input_image < m_threshold;
    Scalar mean_above = mean(input_image, above);
    Scalar mean_below = mean(input_image, below);
    m_threshold = cvRound(((mean_above[0]+mean_below[0])/2));

  } while(old_threshold != m_threshold);
}


void Segmenting::setAdapt(const bool adapt)
{
  m_adapt = adapt;
  process();
}

void Segmenting::setBackground(const Background bg)
{
  m_background = bg;
  process();
}
