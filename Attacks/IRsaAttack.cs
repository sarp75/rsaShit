using rsaShit.Core;

namespace rsaShit.Attacks;

public interface IRsaAttack {
	bool CanExecute(RSAState state);
	void Execute(RSAState state);
	string Name { get; }
}
