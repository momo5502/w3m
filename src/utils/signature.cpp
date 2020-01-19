#include <std_include.hpp>
#include "signature.hpp"
#include "loader/loader.hpp"

namespace utils::hook
{
	void signature::load_pattern(const std::string& pattern)
	{
		this->mask_.clear();
		this->pattern_.clear();
		
		uint8_t nibble = 0;
		auto has_nibble = false;
		
		for(auto val : pattern)
		{
			if (val == ' ') continue;
			if(val == '?')
			{
				this->mask_.push_back(val);
				this->pattern_.push_back(0);
			}
			else
			{
				if((val < '0' || val > '9') && (val < 'A' || val > 'F') && (val < 'a' || val> 'f'))
				{
					throw std::runtime_error("Invalid pattern");
				}
				
				char str[] = { val, 0 };
				const auto current_nibble = static_cast<uint8_t>(strtol(str, nullptr, 16));

				if(!has_nibble)
				{
					has_nibble = true;
					nibble = current_nibble;
				}
				else
				{
					has_nibble = false;
					const uint8_t byte = current_nibble | (nibble << 4);

					this->mask_.push_back('x');
					this->pattern_.push_back(byte);
				}
			}
		}

		if(has_nibble)
		{
			throw std::runtime_error("Invalid pattern");
		}
	}

	std::vector<size_t> signature::process_range(void* start, size_t length)
	{
	}

	signature::signature_result signature::process()
	{
		std::vector<size_t> result;
		
		for (size_t i = 0; i < this->length_; ++i)
		{
			const auto address = this->start_ + i;

			size_t j = 0;
			for (; j < this->mask_.size(); ++j)
			{
				if (this->mask_[j] != '?' && this->pattern_[j] != address[j])
				{
					break;
				}
			}

			if(j == this->mask_.size())
			{
				result.push_back(size_t(address));
			}
		}

		return { std::move(result) };
	}

	signature::signature_result signature::process_parallel()
	{
	}
}
