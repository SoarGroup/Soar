/*
 * semantic_memory_math_queries.h
 *
 *  Created on: Feb 12, 2014
 *      Author: alex.nickels
 */

#ifndef SEMANTIC_MEMORY_MATH_QUERIES_H_
#define SEMANTIC_MEMORY_MATH_QUERIES_H_

//#include <limits>
#undef min
#undef max

class MathQuery
{
	public:
		virtual ~MathQuery() {}

		//Use these to indicate if the current value is a potential new best match
		virtual bool valueIsAcceptable(int64_t value)=0;
		virtual bool valueIsAcceptable(double value)=0;
		//Use this to record things like the new max values
		virtual void commit()=0;
		virtual void rollback()=0;
};

class MathQueryLess: public MathQuery
{
	private:
		double doubleValue;
		int64_t longValue;
		bool isDouble;
	public:
		MathQueryLess(double value)
		{
			doubleValue = value;
			longValue = 0;
			isDouble = true;
		}

		MathQueryLess(int64_t value)
		{
			doubleValue = 0;
			longValue = value;
			isDouble = false;
		}

		bool valueIsAcceptable(double value)
		{
			if(isDouble)
			{
				return value < doubleValue;
			}
			return value < longValue;
		}

		bool valueIsAcceptable(int64_t value)
		{
			if(isDouble)
			{
				return value < doubleValue;
			}
			return value < longValue;
		}

		//There is no running data in this query
		void commit(){}
		void rollback(){}
};

class MathQueryGreater: public MathQuery
{
	private:
		double doubleValue;
		int64_t longValue;
		bool isDouble;
	public:
		MathQueryGreater(double value)
		{
			doubleValue = value;
			longValue = 0;
			isDouble = true;
		}

		MathQueryGreater(int64_t value)
		{
			doubleValue = 0;
			longValue = value;
			isDouble = false;
		}

		bool valueIsAcceptable(double value)
		{
			if(isDouble)
			{
				return value > doubleValue;
			}
			return value > longValue;
		}

		bool valueIsAcceptable(int64_t value)
		{
			if(isDouble)
			{
				return value > doubleValue;
			}
			return value > longValue;
		}

		//There is no running data in this query
		void commit(){}
		void rollback(){}
};
class MathQueryLessOrEqual: public MathQuery
{
	private:
		double doubleValue;
		int64_t longValue;
		bool isDouble;
	public:
		MathQueryLessOrEqual(double value)
		{
			doubleValue = value;
			longValue = 0;
			isDouble = true;
		}

		MathQueryLessOrEqual(int64_t value)
		{
			doubleValue = 0;
			longValue = value;
			isDouble = false;
		}

		bool valueIsAcceptable(double value)
		{
			if(isDouble)
			{
				return value <= doubleValue;
			}
			return value <= longValue;
		}

		bool valueIsAcceptable(int64_t value)
		{
			if(isDouble)
			{
				return value <= doubleValue;
			}
			return value <= longValue;
		}

		//There is no running data in this query
		void commit(){}
		void rollback(){}
};

class MathQueryGreaterOrEqual: public MathQuery
{
	private:
		double doubleValue;
		int64_t longValue;
		bool isDouble;
	public:
		MathQueryGreaterOrEqual(double value)
		{
			doubleValue = value;
			longValue = 0;
			isDouble = true;
		}

		MathQueryGreaterOrEqual(int64_t value)
		{
			doubleValue = 0;
			longValue = value;
			isDouble = false;
		}

		bool valueIsAcceptable(double value)
		{
			if(isDouble)
			{
				return value >= doubleValue;
			}
			return value >= longValue;
		}

		bool valueIsAcceptable(int64_t value)
		{
			if(isDouble)
			{
				return value >= doubleValue;
			}
			return value >= longValue;
		}

		//There is no running data in this query
		void commit(){}
		void rollback(){}
};

class MathQueryMax: public MathQuery
{
	private:
		double doubleValue;
		double stagedDoubleValue;
		int64_t longValue;
		int64_t stagedLongValue;

		void stageDouble(double d)
		{
			if(d > stagedDoubleValue)
			{
				stagedDoubleValue = d;
			}
		}
		void stageLong(int64_t l)
		{
			if(l > stagedLongValue)
			{
				stagedLongValue = l;
			}
		}
	public:
		MathQueryMax()
		{
			doubleValue = std::numeric_limits<double>::min();
			stagedDoubleValue = std::numeric_limits<double>::min();
			longValue = std::numeric_limits<int64_t>::min();
			stagedLongValue = std::numeric_limits<int64_t>::min();
		}

		bool valueIsAcceptable(double value)
		{
			if(value > doubleValue && value > longValue)
			{
				stageDouble(value);
				return true;
			}
			return false;
		}

		bool valueIsAcceptable(int64_t value)
		{
			if(value > doubleValue && value > longValue)
			{
				stageLong(value);
				return true;
			}
			return false;
		}

		void commit()
		{
			doubleValue = stagedDoubleValue;
			longValue = stagedLongValue;
		}
		void rollback()
		{
			stagedDoubleValue = doubleValue;
			stagedLongValue = longValue;
		}
};

class MathQueryMin: public MathQuery
{
	private:
		double doubleValue;
		double stagedDoubleValue;
		int64_t longValue;
		int64_t stagedLongValue;

		void stageDouble(double d)
		{
			if(d < stagedDoubleValue)
			{
				stagedDoubleValue = d;
			}
		}
		void stageLong(int64_t l)
		{
			if(l < stagedLongValue)
			{
				stagedLongValue = l;
			}
		}
	public:
		MathQueryMin()
		{
			doubleValue = std::numeric_limits<double>::max();
			stagedDoubleValue = std::numeric_limits<double>::max();
			longValue = std::numeric_limits<int64_t>::max();
			stagedLongValue = std::numeric_limits<int64_t>::max();
		}

		bool valueIsAcceptable(double value)
		{
			if(value < doubleValue && value < longValue)
			{
				stageDouble(value);
				return true;
			}
			return false;
		}

		bool valueIsAcceptable(int64_t value)
		{
			if(value < doubleValue && value < longValue)
			{
				stageLong(value);
				return true;
			}
			return false;
		}

		void commit()
		{
			doubleValue = stagedDoubleValue;
			longValue = stagedLongValue;
		}
		void rollback()
		{
			stagedDoubleValue = doubleValue;
			stagedLongValue = longValue;
		}
};

#endif /* SEMANTIC_MEMORY_MATH_QUERIES_H_ */
