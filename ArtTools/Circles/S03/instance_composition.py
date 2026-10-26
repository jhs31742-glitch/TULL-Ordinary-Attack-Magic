"""Development DATA expansion and static validation; no engine/runtime implementation."""
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / 'generator/magician_instances.candidate.json'


def load_composition():
    config = json.loads(CONFIG.read_text(encoding='utf-8'))
    identities = config['circles']
    assert config['status'] == 'CANDIDATE VALUES ONLY'
    assert [c['target_scale'] for c in identities] == [1, 1.35, .70, 1.55, 1.05, .65, 1.40, .90]
    assert [c['id'] for c in identities] == [f'C{i}' for i in range(1, 9)]
    assert len({c['instance_key'] for c in identities}) == 8
    assert [t['mp'] for t in config['tiers']] == [100, 300, 600, 1200, 2000, 3000, 4000, 5000]
    expanded = []
    for tier in config['tiers']:
        n = tier['tier']
        assert tier['visible_ids'] == [c['id'] for c in identities[:n]]
        assert len(tier['offset_y']) == n and tier['offset_y'][0] == 0
        assert all(a < b for a, b in zip(tier['offset_y'], tier['offset_y'][1:]))
        instances = []
        for i, identity in enumerate(identities):
            visible = i < n
            instances.append({**identity, 'template_id': config['template']['id'],
                              'visible': visible,
                              'translation': [0, tier['offset_y'][i], 0] if visible else None,
                              'scale': identity['target_scale'], 'rotation_y_degrees': 0})
        expanded.append({**tier, 'instances': instances})
    anchor = expanded[0]['instances'][0]
    assert anchor['translation'] == [0, 0, 0] and anchor['scale'] == 1
    assert all(t['instances'][0] == anchor for t in expanded)
    assert expanded[1]['offset_y'] == [0, 96], 'T2 retains uncompressed candidate baseline'
    for previous, current in zip(expanded[1:], expanded[2:]):
        for i in range(1, previous['tier']):
            a, b = previous['instances'][i], current['instances'][i]
            assert b['translation'][1] < a['translation'][1]
            assert b['scale'] == a['scale'], 'Compaction must not change size identity'
    for t in expanded:
        for c in t['instances']:
            assert math.isfinite(c['scale']) and c['scale'] > 0
            if c['visible']:
                assert all(map(math.isfinite, c['translation']))
    return config, expanded


def write_expanded(config, tiers):
    path = ROOT / 'reports/MAGICIAN_TIER_PREVIEW_VALUES.json'
    path.write_text(json.dumps({'schema': config['schema'], 'status': config['status'],
                                'architecture': config['architecture'], 'tiers': tiers}, indent=2), encoding='utf-8')
    return path


if __name__ == '__main__':
    config, tiers = load_composition()
    print(write_expanded(config, tiers))
